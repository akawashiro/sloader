#include "dyn_loader.h"

#include <asm/prctl.h>
#include <fcntl.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include <cstring>
#include <map>

#include "libc_mapping.h"

extern thread_local unsigned long sloader_dummy_to_secure_tls_space[];
extern unsigned long sloader_tls_offset;
void write_sloader_dummy_to_secure_tls_space();

namespace {

void read_ldsoconf_dfs(std::vector<std::filesystem::path>& res, const std::string& filename) {
    std::ifstream f;
    f.open(filename);

    // TODO: Workaround not to load i386 libs.
    if (!f || filename.find("i386") != std::string::npos || filename.find("lib32") != std::string::npos) {
        return;
    }
    std::string head;
    while (f >> head) {
        if (head.substr(0, 1) == "#") {
            std::string comment;
            std::getline(f, comment);
        } else if (head == "include") {
            std::string descendants;
            f >> descendants;

            glob_t globbuf;
            glob(descendants.c_str(), 0, NULL, &globbuf);
            for (size_t i = 0; i < globbuf.gl_pathc; i++) {
                read_ldsoconf_dfs(res, globbuf.gl_pathv[i]);
            }
            globfree(&globbuf);
        } else {
            res.push_back(head);
        }
    }
}

}  // namespace

std::vector<std::filesystem::path> read_ldsoconf() {
    std::vector<std::filesystem::path> res;
    read_ldsoconf_dfs(res, "/etc/ld.so.conf");

    return res;
}

ELFBinary::ELFBinary(const std::filesystem::path path) : path_(path) {
    int fd = open(path_.c_str(), O_RDONLY);
    CHECK(fd >= 0);

    size_t size = lseek(fd, 0, SEEK_END);
    CHECK_GT(size, 8UL + 16UL);

    size_t mapped_size = (size + 0xfff) & ~0xfff;

    file_base_addr_ = (char*)mmap(NULL, mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
    CHECK(file_base_addr_ != MAP_FAILED);

    ehdr_ = *reinterpret_cast<Elf64_Ehdr*>(file_base_addr_);
    for (uint16_t i = 0; i < ehdr_.e_phnum; i++) {
        Elf64_Phdr ph = *reinterpret_cast<Elf64_Phdr*>(file_base_addr_ + ehdr_.e_phoff + i * ehdr_.e_phentsize);
        file_phdrs_.emplace_back(ph);

        if (ph.p_type == PT_DYNAMIC) {
            LOG(INFO) << "Found PT_DYNAMIC";
            file_dynamic_ = ph;
        } else if (ph.p_type == PT_TLS) {
            LOG(INFO) << "Found PT_TLS";
            has_tls_ = true;
            file_tls_ = ph;
        }
    }
}

void ELFBinary::Load(Elf64_Addr base_addr, std::ofstream& map_file) {
    base_addr_ = base_addr;
    end_addr_ = base_addr_;

    LOG(INFO) << "Load start " << path_;

    for (auto ph : file_phdrs_) {
        if (ph.p_type != PT_LOAD) {
            continue;
        }
        LOG(INFO) << LOG_BITS(reinterpret_cast<void*>(ph.p_vaddr)) << LOG_BITS(ph.p_memsz);
        void* mmap_start = reinterpret_cast<void*>(((ph.p_vaddr + base_addr) & (~(0xfff))));
        void* mmap_end = reinterpret_cast<void*>((((ph.p_vaddr + ph.p_memsz + base_addr) + 0xfff) & (~(0xfff))));
        end_addr_ = reinterpret_cast<Elf64_Addr>(mmap_end);
        size_t mmap_size = reinterpret_cast<size_t>(mmap_end) - reinterpret_cast<size_t>(mmap_start);
        int flags = 0;
        std::string flags_str = "";
        if (ph.p_flags & PF_R) {
            flags |= PROT_READ;
            flags_str += "r";
        } else {
            flags_str += "_";
        }
        if ((ph.p_flags & PF_W) || true) {  // TODO: We need to write contents after mmap.
            flags |= PROT_WRITE;
            flags_str += "w";
        } else {
            flags_str += "_";
        }
        if (ph.p_flags & PF_X) {
            flags |= PROT_EXEC;
            flags_str += "x";
        } else {
            flags_str += "_";
        }

        char* p = reinterpret_cast<char*>(mmap(mmap_start, mmap_size, flags, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
        LOG(INFO) << "mmap: " << LOG_KEY(path_) << LOG_BITS(p) << LOG_BITS(mmap_start) << LOG_BITS(ph.p_vaddr)
                  << "errno = " << std::strerror(errno);
        CHECK_EQ(mmap_start, +p);
        CHECK_LE(reinterpret_cast<Elf64_Addr>(mmap_start), ph.p_vaddr + base_addr);
        CHECK_LE(ph.p_vaddr + base_addr + ph.p_memsz, reinterpret_cast<Elf64_Addr>(mmap_end));
        LOG(INFO) << LOG_BITS(mmap_start) << LOG_BITS(reinterpret_cast<size_t>(file_base_addr_ + ph.p_offset)) << LOG_BITS(ph.p_filesz);
        map_file << path().string() << " " << HexString(ph.p_offset, 16) << "-" << HexString(ph.p_offset + ph.p_filesz, 16)
                 << " " << flags_str << " " << HexString(ph.p_filesz, 16) << " => " << HexString(mmap_start, 16) << "-"
                 << HexString(mmap_end, 16) << std::endl;
        memcpy(reinterpret_cast<void*>(ph.p_vaddr + base_addr), file_base_addr_ + ph.p_offset, ph.p_filesz);
    }
    LOG(INFO) << "Load end";

    LOG(INFO) << "ParseDynamic start";
    ParseDynamic();
    LOG(INFO) << "ParseDynamic end";
}

void ELFBinary::ParseDynamic() {
    // Must mmap PT_LOADs before call ParseDynamic.
    CHECK_NE(base_addr_, 0UL);

    const size_t dyn_size = sizeof(Elf64_Dyn);
    CHECK_EQ(file_dynamic_.p_filesz % dyn_size, 0U);

    // Search DT_STRTAB at first.
    for (size_t i = 0; i < file_dynamic_.p_filesz / dyn_size; ++i) {
        Elf64_Dyn* dyn = reinterpret_cast<Elf64_Dyn*>(base_addr_ + file_dynamic_.p_vaddr + dyn_size * i);
        LOG(INFO) << LOG_KEY(dyn);
        if (dyn->d_tag == DT_STRTAB) {
            LOG(INFO) << "Found DT_STRTAB";
            strtab_ = reinterpret_cast<char*>(dyn->d_un.d_ptr + base_addr_);
        } else if (dyn->d_tag == DT_STRSZ) {
            LOG(INFO) << "Found DT_STRSZ";
            strsz_ = dyn->d_un.d_val;
        }
    }

    CHECK_NE(strtab_, nullptr);

    for (size_t i = 0; i < file_dynamic_.p_filesz / dyn_size; ++i) {
        Elf64_Dyn* dyn = reinterpret_cast<Elf64_Dyn*>(base_addr_ + file_dynamic_.p_vaddr + dyn_size * i);
        if (dyn->d_tag == DT_NEEDED) {
            std::string needed = strtab_ + dyn->d_un.d_val;
            neededs_.emplace_back(needed);
            LOG(INFO) << LOG_KEY(needed);
        } else if (dyn->d_tag == DT_RUNPATH) {
            // TODO: Handle relative path
            runpath_ = strtab_ + dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_RPATH) {
            // TODO: Handle relative path
            rpath_ = strtab_ + dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_RELA) {
            LOG(INFO) << "Found DT_RELA";
            rela_ = reinterpret_cast<Elf64_Rela*>(base_addr_ + dyn->d_un.d_val);
        } else if (dyn->d_tag == DT_RELASZ) {
            relasz_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_RELAENT) {
            relaent_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_RELACOUNT) {
            relacount_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_JMPREL) {
            jmprel_ = reinterpret_cast<Elf64_Rela*>(base_addr_ + dyn->d_un.d_val);
        } else if (dyn->d_tag == DT_PLTRELSZ) {
            pltrelsz_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_PLTREL) {
            pltrel_ = dyn->d_un.d_val;
            CHECK(pltrel_ == DT_RELA || pltrel_ == DT_REL);
            pltrelent_ = (pltrel_ == DT_RELA) ? sizeof(Elf64_Rela) : sizeof(Elf64_Rel);
        } else if (dyn->d_tag == DT_SYMTAB) {
            symtab_ = reinterpret_cast<Elf64_Sym*>(base_addr_ + dyn->d_un.d_val);
        } else if (dyn->d_tag == DT_SYMENT) {
            syment_ = dyn->d_un.d_val;
            CHECK_EQ(syment_, sizeof(Elf64_Sym));
        } else if (dyn->d_tag == DT_INIT) {
            init_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_FINI) {
            fini_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_INIT_ARRAY) {
            init_array_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_INIT_ARRAYSZ) {
            init_arraysz_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_FINI_ARRAY) {
            fini_array_ = dyn->d_un.d_val;
        } else if (dyn->d_tag == DT_FINI_ARRAYSZ) {
            fini_arraysz_ = dyn->d_un.d_val;
        }
    }

    LOG(INFO) << LOG_KEY(relasz_) << LOG_KEY(relaent_) << LOG_KEY(relacount_);
    if (rela_ != nullptr) {
        CHECK_EQ(relasz_ % relaent_, 0UL);
        Elf64_Rela* r = rela_;
        for (size_t i = 0; i < relasz_ / relaent_; i++, r++) {
            relas_.emplace_back(*r);
            LOG(INFO) << ShowRela(relas_.back());
        }
    }

    LOG(INFO) << LOG_KEY(pltrelsz_) << LOG_KEY(pltrelent_);
    if (jmprel_ != nullptr) {
        CHECK_EQ(pltrelsz_ % pltrelent_, 0UL);
        CHECK_EQ(pltrel_, static_cast<unsigned long>(DT_RELA));
        Elf64_Rela* r = jmprel_;
        for (size_t i = 0; i < pltrelsz_ / pltrelent_; i++, r++) {
            pltrelas_.emplace_back(*r);
            LOG(INFO) << ShowRela(pltrelas_.back());
        }
    }

    if (symtab_ != nullptr) {
        Elf64_Sym* s = symtab_;
        symtabs_.emplace_back(*s);
        s++;

        // TODO: This is a hack. Listing up all symbols is always difficult.
        while (0 <= s->st_name && s->st_name < strsz_) {
            symtabs_.emplace_back(*s);
            s++;
        }

        for (const auto& s : symtabs_) {
            LOG(INFO) << LOG_KEY(s.st_name);
            LOG(INFO) << ShowSym(s, strtab_);
        }
    }
}

const Elf64_Addr ELFBinary::GetSymbolAddr(const size_t symbol_index) {
    CHECK_LT(symbol_index, symtabs().size());
    return symtabs()[symbol_index].st_value + base_addr();
}

DynLoader::DynLoader(const std::filesystem::path& main_path, const std::vector<std::string>& args, const std::vector<std::string>& envs)
    : main_path_(main_path), args_(args), envs_(envs) {
    Elf64_Addr base_addr = 0x40'0000;
    binaries_.emplace_back(ELFBinary(main_path));

    std::ofstream map_file(std::getenv("SLOADER_MAP_FILE") == nullptr ? "/tmp/sloader_map" : std::getenv("SLOADER_MAP_FILE"));
    binaries_.back().Load(base_addr, map_file);
    base_addr = (binaries_.back().end_addr() + (0x400000 - 1)) / 0x400000 * 0x400000;

    std::queue<std::tuple<std::string, std::optional<std::filesystem::path>, std::optional<std::filesystem::path>>> queue;
    std::set<std::string> loaded;

    for (const auto& n : binaries_.back().neededs()) {
        queue.push(std::make_tuple(n, binaries_.back().runpath(), binaries_.back().rpath()));
    }

    // Search depending sos.
    while (!queue.empty()) {
        const auto [library_name, runpath, rpath] = queue.front();
        queue.pop();

        if (loaded.count(library_name) != 0) continue;
        loaded.insert(library_name);

        // Skip dynamic loader and libc.so
        if (library_name.find("ld-linux") != std::string::npos || library_name.find("libc.so") != std::string::npos) {
            LOG(INFO) << "Skip " << library_name;
            continue;
        }

        const auto library_path = FindLibrary(library_name, runpath, rpath);
        binaries_.emplace_back(ELFBinary(library_path));
        binaries_.back().Load(base_addr, map_file);
        base_addr = (binaries_.back().end_addr() + (0x400000 - 1)) / 0x400000 * 0x400000;
        for (const auto& n : binaries_.back().neededs()) {
            queue.push(std::make_tuple(n, binaries_.back().runpath(), binaries_.back().rpath()));
        }
    }

    Relocate();

    Execute(args_, envs_);
}

// To assign variables of stack, stack_num and entry to %rdi, %rsi and %rdx
// I use the calling convention. For details, see A.2.1 Calling Conventions
// in https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf. Of
// course, compiler must not inline this function.
void __attribute__((noinline)) DynLoader::ExecuteCore(uint64_t* stack, size_t stack_num, uint64_t entry) {
    for (size_t i = 0; i < stack_num; i++) {
        asm volatile("pushq %0" ::"m"(*(stack + i)));
    }

    asm volatile("jmp *%0" ::"r"(entry));
}

// Copied from glibc
// Type of a constructor function, in DT_INIT, DT_INIT_ARRAY, DT_PREINIT_ARRAY.
// argc, argv, env
typedef void (*dl_init_t)(int, char**, char**);

void DynLoader::Execute(std::vector<std::string> args, std::vector<std::string> envs) {
    // TODO: Pass arguments
    char* argv[] = {const_cast<char*>(main_path_.c_str())};
    char** env = reinterpret_cast<char**>(malloc(sizeof(const char*) * envs.size()));
    for (size_t i = 0; i < envs.size(); i++) {
        env[i] = const_cast<char*>(envs[i].c_str());
    }

    for (int i = binaries_.size() - 1; 0 <= i; i--) {
        if (binaries_[i].init() != 0) {
            reinterpret_cast<dl_init_t>(binaries_[i].init() + binaries_[i].base_addr())(1, argv, env);
        }
        if (binaries_[i].init_arraysz() != 0) {
            CHECK_EQ(binaries_[i].init_arraysz() % 8, 0UL);  // Assume 64bits
            LOG(INFO) << LOG_BITS(i) << LOG_BITS(binaries_[i].init_arraysz());
            Elf64_Addr* init_array_funs =
                reinterpret_cast<Elf64_Addr*>((reinterpret_cast<char**>(binaries_[i].init_array() + binaries_[i].base_addr())));

            for (long unsigned int j = 0; j < binaries_[i].init_arraysz() / 8; j++) {
                LOG(INFO) << LOG_KEY(binaries_[i].filename()) << LOG_BITS(binaries_[i].init_array()) << LOG_BITS(init_array_funs[j])
                          << LOG_BITS(binaries_[i].base_addr()) << LOG_BITS(init_array_funs[j] + binaries_[i].base_addr());
                if (reinterpret_cast<dl_init_t>(init_array_funs[j]) == nullptr) {
                    LOG(FATAL) << LOG_BITS(init_array_funs[j]);
                    break;
                }
                reinterpret_cast<dl_init_t>(init_array_funs[j])(1, argv, env);
            }
        }
    }

    unsigned long at_random = getauxval(AT_RANDOM);
    unsigned long at_pagesz = getauxval(AT_PAGESZ);
    CHECK_NE(at_random, 0UL);
    LOG(INFO) << LOG_BITS(at_random) << LOG_BITS(at_pagesz);

    // Some commented out auxiliary values because they are not appropriate
    // as loading programs. These values are for sloader itself.
    std::vector<unsigned long> aux_types{AT_IGNORE,
                                         // AT_EXECFD,
                                         // AT_PHDR,
                                         AT_PHENT,
                                         // AT_PHNUM,
                                         AT_PAGESZ,
                                         // AT_BASE,
                                         AT_FLAGS,
                                         // AT_ENTRY,
                                         AT_NOTELF, AT_UID, AT_EUID, AT_GID, AT_EGID, AT_CLKTCK, AT_PLATFORM, AT_HWCAP, AT_FPUCW,
                                         AT_DCACHEBSIZE, AT_ICACHEBSIZE, AT_UCACHEBSIZE, AT_IGNOREPPC, AT_SECURE, AT_BASE_PLATFORM,
                                         AT_RANDOM, AT_HWCAP2,
                                         // AT_EXECFN,
                                         AT_SYSINFO, AT_SYSINFO_EHDR, AT_L1I_CACHESHAPE, AT_L1D_CACHESHAPE, AT_L2_CACHESHAPE,
                                         AT_L3_CACHESHAPE, AT_L1I_CACHESIZE, AT_L1I_CACHEGEOMETRY, AT_L1D_CACHESIZE, AT_L1D_CACHEGEOMETRY,
                                         AT_L2_CACHESIZE, AT_L2_CACHEGEOMETRY, AT_L3_CACHESIZE, AT_L3_CACHEGEOMETRY, AT_MINSIGSTKSZ};

    std::vector<std::pair<unsigned long, unsigned long>> aux_tvs;
    for (size_t i = 0; i < aux_types.size(); i++) {
        unsigned long v = getauxval(aux_types[i]);
        if (v != 0) {
            aux_tvs.emplace_back(std::make_pair(aux_types[i], v));
            LOG(INFO) << LOG_BITS(aux_types[i]) << LOG_BITS(v);
        }
    }

    // See http://articles.manugarg.com/aboutelfauxiliaryvectors.html for
    // the stack layout padding.
    //
    // 4 words padding
    // 0
    // AT_NULL
    // auxs
    // NULL
    // envs
    // argv[argc] (must be null)
    // argv[0] = filename
    // argc
    size_t stack_index = 0;
    size_t stack_num = 4 + 2 + 2 * aux_tvs.size() + 1 + envs.size() + 2 + args.size();
    size_t stack_size = sizeof(uint64_t) * stack_num;
    unsigned long* stack = reinterpret_cast<uint64_t*>(malloc(stack_size));
    memset(stack, 0, stack_size);

    // 4 words padding
    stack_index += 4;

    // First two elements are 0 and AT_NULL.
    stack_index += 2;

    // auxs
    for (size_t i = 0; i < aux_tvs.size(); i++) {
        *(stack + stack_index) = aux_tvs[i].second;
        stack_index++;
        *(stack + stack_index) = aux_tvs[i].first;
        stack_index++;
    }

    // End of environment variables
    stack_index++;

    // Environment variables
    for (size_t i = 0; i < envs.size(); i++) {
        *(stack + stack_index) = reinterpret_cast<uint64_t>(envs[envs.size() - 1 - i].c_str());
        stack_index++;
    }

    // argv[argc]
    stack_index++;

    for (size_t i = 0; i < args.size(); i++) {
        LOG(INFO) << (args[i]);
        *(stack + stack_index) = reinterpret_cast<uint64_t>(args[args.size() - 1 - i].c_str());
        stack_index++;
    }

    // argc
    *(stack + stack_index) = args.size();
    stack_index++;

    CHECK_EQ(stack_index, stack_num);

    LOG(INFO) << LOG_BITS(binaries_[0].ehdr().e_entry + binaries_[0].base_addr()) << std::endl;

    // TLS initialization
    // TODO: We support only static TLS i.e. don't support dlopen.
    //
    // =========== address ==========>
    //
    // tls_block (= sloader_dummy_to_secure_tls_space)                       tls_block + TLS_SPACE_FOR_LOADEE
    // |                                                                                                   |
    // v                                                                                                   v
    // [.tdata of binaries_[n]] [.tbss of binaries_[n]] ... [.tdata of binaries_[0]] [.tbss of binaries_[0]]

    {
        size_t tls_block_size = 0;
        for (const ELFBinary& b : binaries_) {
            if (b.has_tls()) {
                tls_block_size += b.file_tls().p_memsz;
            }
        }
        CHECK_LE(tls_block_size, 4096UL);
    }

    void* tls_block = sloader_dummy_to_secure_tls_space;

    // Copy .tdata and .tbss of each binary
    for (const ELFBinary& b : binaries_) {
        if (b.has_tls()) {
            LOG(INFO) << LOG_BITS(reinterpret_cast<uint64_t>(tls_block)) << LOG_BITS(reinterpret_cast<uint64_t>(b.file_tls().p_memsz));
            LOG(INFO) << LOG_BITS(reinterpret_cast<uint64_t>(b.file_tls().p_memsz))
                      << LOG_BITS(reinterpret_cast<uint64_t>(b.file_tls().p_filesz)) << LOG_KEY(b.path());

            // Set .tdata
            memcpy(reinterpret_cast<char*>(tls_block) + sloader_tls_offset - b.file_tls().p_memsz,
                   reinterpret_cast<const void*>(b.base_addr() + b.file_tls().p_vaddr), b.file_tls().p_memsz);
            // Set .tbss
            memset(reinterpret_cast<char*>(tls_block) + sloader_tls_offset - (b.file_tls().p_memsz - b.file_tls().p_filesz), 0x0,
                   b.file_tls().p_memsz - b.file_tls().p_filesz);

            *reinterpret_cast<void**>(reinterpret_cast<char*>(tls_block) + sloader_tls_offset) =
                reinterpret_cast<char*>(tls_block) + sloader_tls_offset;
            sloader_tls_offset -= b.file_tls().p_memsz;
        }
    }

    ExecuteCore(stack, stack_num, binaries_[0].ehdr().e_entry + binaries_[0].base_addr());

    free(stack);
    LOG(INFO) << "Execute end";
}

std::filesystem::path DynLoader::FindLibrary(std::string library_name, std::optional<std::filesystem::path> runpath,
                                             std::optional<std::filesystem::path> rpath) {
    std::vector<std::filesystem::path> library_directory;

    std::string sloader_library_path(std::getenv("SLOADER_LIBRARY_PATH") == nullptr ? "" : std::getenv("SLOADER_LIBRARY_PATH"));
    if (!sloader_library_path.empty()) {
        library_directory.emplace_back(sloader_library_path);
    }

    if (runpath) {
        library_directory.emplace_back(runpath.value());
    }
    if (rpath) {
        library_directory.emplace_back(rpath.value());
    }
    const auto ldsoconfs = read_ldsoconf();
    library_directory.insert(library_directory.end(), ldsoconfs.begin(), ldsoconfs.end());
    library_directory.emplace_back("/lib");
    library_directory.emplace_back("/usr/lib");
    library_directory.emplace_back("/usr/lib64");

    for (const auto& d : library_directory) {
        std::filesystem::path p = d / library_name;
        if (std::filesystem::exists(p)) {
            LOG(INFO) << LOG_KEY(p);
            return p;
        }
    }
    LOG(FATAL) << "Cannot find " << LOG_KEY(library_name);
    std::abort();
}

// Search the first defined symbol
// Return pair of the index of ELFBinary and the index of the Elf64_Sym
// TODO: Consider version information
// TODO: Return ELFBinary and Elf64_Sym theirselves
std::optional<std::pair<size_t, size_t>> DynLoader::SearchSym(const std::string& name, bool skip_main = false) {
    LOG(INFO) << "========== SearchSym " << name << "==========";
    // binaries_[0] is the executable itself. We should skip it.
    // TODO: Add reference here.
    for (size_t i = skip_main ? 1 : 0; i < binaries_.size(); i++) {
        for (size_t j = 0; j < binaries_[i].symtabs().size(); j++) {
            const Elf64_Sym& s = binaries_[i].symtabs()[j];
            std::string n = s.st_name + binaries_[i].strtab();
            if (n == name && s.st_shndx != SHN_UNDEF) {
                LOG(INFO) << "Found " << name << " at index " << j << " of " << binaries_[i].path();
                return std::make_optional(std::make_pair(i, j));
            }
        }
    }
    for (size_t i = skip_main ? 1 : 0; i < binaries_.size(); i++) {
        for (size_t j = 0; j < binaries_[i].symtabs().size(); j++) {
            const Elf64_Sym& s = binaries_[i].symtabs()[j];
            std::string n = s.st_name + binaries_[i].strtab();
            if (n == name && s.st_shndx == SHN_UNDEF && ELF64_ST_BIND(s.st_info) == STB_WEAK) {
                LOG(WARNING) << "Found " << name << " at index as an weak symbol " << j << " of " << binaries_[i].path();
                return std::make_optional(std::make_pair(i, j));
            }
        }
    }
    return std::nullopt;
}

Elf64_Addr DynLoader::TLSSymOffset(const std::string& name) {
    // Intentional use of underflow
    Elf64_Addr offset = 0x0;
    for (size_t i = 0; i < binaries_.size(); i++) {
        for (size_t j = 0; j < binaries_[i].symtabs().size(); j++) {
            Elf64_Sym s = binaries_[i].symtabs()[j];
            std::string n = s.st_name + binaries_[i].strtab();
            if (n == name && s.st_shndx != SHN_UNDEF && ELF64_ST_TYPE(s.st_info) == STT_TLS) {
                Elf64_Addr o = offset - binaries_[i].file_tls().p_memsz + s.st_value;
                LOG(INFO) << "Found " << name << " at index " << j << " of " << binaries_[i].path() << LOG_BITS(o);
                return offset - binaries_[i].file_tls().p_memsz + s.st_value;
            }
        }
        if (binaries_[i].has_tls()) {
            offset -= binaries_[i].file_tls().p_memsz;
        }
    }

    // Workaround for TLS variable in libc.so such as errno
    if(name == "errno"){
        return (reinterpret_cast<const char*>(sloader_dummy_to_secure_tls_space) + 4096 - reinterpret_cast<const char*>(&errno));
    }
    LOG(FATAL) << "Cannot find " << name;
}

void DynLoader::Relocate() {
    for (const auto& bin : binaries_) {
        LOG(INFO) << bin.path();

        std::vector<Elf64_Rela> relas = bin.pltrelas();
        // TODO: Use std::copy?
        for (const auto r : bin.relas()) {
            relas.emplace_back(r);
        }

        for (const auto& r : relas) {
            CHECK_LT(ELF64_R_SYM(r.r_info), bin.symtabs().size());
            Elf64_Sym s = bin.symtabs()[ELF64_R_SYM(r.r_info)];
            std::string name = s.st_name + bin.strtab();
            LOG(INFO) << ShowRela(r) << LOG_KEY(name);

            switch (ELF64_R_TYPE(r.r_info)) {
                case R_X86_64_GLOB_DAT:
                case R_X86_64_JUMP_SLOT: {
                    LOG(INFO) << ShowRelocationType(ELF64_R_TYPE(r.r_info));
                    const auto opt = SearchSym(name);
                    Elf64_Addr sym_addr;

                    if (libc_mapping::sloader_libc_map.find(name) != libc_mapping::sloader_libc_map.end()) {
                        sym_addr = libc_mapping::sloader_libc_map[name];
                    } else if (opt) {
                        const auto [bin_index, sym_index] = opt.value();
                        sym_addr = binaries_[bin_index].GetSymbolAddr(sym_index);
                    } else {
                        LOG(WARNING) << "Cannot find " << name << LOG_KEY(bin.path());
                        break;
                    }

                    Elf64_Addr* reloc_addr = reinterpret_cast<Elf64_Addr*>(bin.base_addr() + r.r_offset);
                    LOG(INFO) << LOG_KEY(reloc_addr) << LOG_BITS(*reloc_addr) << LOG_BITS(sym_addr);
                    // TODO: Although glibc add sym_addr to the original value
                    // here
                    // https://github.com/akawashiro/glibc/blob/008003dc6e83439c5e04a744b7fd8197df19096e/sysdeps/x86_64/dl-machine.h#L561,
                    // We just assign it.
                    *reloc_addr = sym_addr;
                    break;
                }
                    // TODO: Is is correct?
                case R_X86_64_IRELATIVE:
                case R_X86_64_RELATIVE: {
                    Elf64_Addr* reloc_addr = reinterpret_cast<Elf64_Addr*>(bin.base_addr() + r.r_offset);
                    *reloc_addr = reinterpret_cast<Elf64_Addr>(bin.base_addr() + r.r_addend);
                    break;
                }
                case R_X86_64_64: {
                    const auto opt = SearchSym(name);
                    Elf64_Addr sym_addr;

                    if (libc_mapping::sloader_libc_map.find(name) != libc_mapping::sloader_libc_map.end()) {
                        sym_addr = libc_mapping::sloader_libc_map[name];
                    } else if (opt) {
                        const auto [bin_index, sym_index] = opt.value();
                        sym_addr = binaries_[bin_index].GetSymbolAddr(sym_index);
                    } else {
                        LOG(WARNING) << "Cannot find " << name << LOG_KEY(bin.path());
                        break;
                    }

                    Elf64_Addr* reloc_addr = reinterpret_cast<Elf64_Addr*>(bin.base_addr() + r.r_offset);

                    // TODO: This is wrong, maybe. What is symbol value?
                    // Elf64_Sym sym = binaries_[bin_index].symtabs()[sym_index];
                    // *reloc_addr = bin.base_addr() + sym.st_value + r.r_addend;
                    *reloc_addr = sym_addr + r.r_addend;
                    break;
                }
                case R_X86_64_TPOFF64: {
                    Elf64_Addr* reloc_addr = reinterpret_cast<Elf64_Addr*>(bin.base_addr() + r.r_offset);
                    Elf64_Addr offset = TLSSymOffset(name);
                    *reloc_addr = offset;
                    break;
                }
                case R_X86_64_DTPMOD64: {
                    Elf64_Addr* reloc_addr = reinterpret_cast<Elf64_Addr*>(bin.base_addr() + r.r_offset);
                    // TODO: Need reference.
                    *reloc_addr = 0x1;
                    break;
                }
                case R_X86_64_DTPOFF64: {
                    break;
                }
                case R_X86_64_COPY: {
                    const auto opt = SearchSym(name, true);
                    void* src;
                    Elf64_Xword size;

                    if (libc_mapping::sloader_libc_map.find(name) != libc_mapping::sloader_libc_map.end()) {
                        src = reinterpret_cast<void*>(libc_mapping::sloader_libc_map[name]);
                        size = 8;
                    } else if (opt) {
                        const auto [bin_index, sym_index] = opt.value();
                        Elf64_Sym sym = binaries_[bin_index].symtabs()[sym_index];
                        src = reinterpret_cast<void*>(binaries_[bin_index].base_addr() + sym.st_value);
                        size = sym.st_size;
                    } else {
                        LOG(FATAL) << "Cannot find " << name;
                        break;
                    }
                    void* dest = reinterpret_cast<void*>(bin.base_addr() + r.r_offset);
                    LOG(INFO) << LOG_BITS(src) << LOG_BITS(dest) << LOG_BITS(*reinterpret_cast<const unsigned long*>(src))
                              << LOG_BITS(size);
                    std::memcpy(dest, src, size);
                    // std::abort();
                    break;
                }
                default: {
                    LOG(FATAL) << "Unsupported! " << ShowRela(r) << std::endl;
                    std::abort();
                    break;
                }
            }
        }
    }
}

std::unique_ptr<DynLoader> MakeDynLoader(const std::filesystem::path& main_path, const std::vector<std::string>& envs,
                                         const std::vector<std::string>& args) {
    // TODO: Remove this call
    write_sloader_dummy_to_secure_tls_space();
    return std::make_unique<DynLoader>(main_path, args, envs);
}
