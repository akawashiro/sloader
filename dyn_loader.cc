#include "dyn_loader.h"

#include <elf.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace {
void read_ldsoconf_dfs(std::vector<std::filesystem::path>& res,
                       const std::string& filename) {
    std::ifstream f;
    f.open(filename);

    // TODO: Workaround not to load i386 libs.
    if (!f || filename.find("i386") != std::string::npos) {
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
            for (int i = 0; i < globbuf.gl_pathc; i++) {
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
    CHECK_GT(size, 8 + 16);

    size_t mapped_size = (size + 0xfff) & ~0xfff;

    file_base_addr_ =
        (char*)mmap(NULL, mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                    MAP_PRIVATE, fd, 0);
    CHECK(file_base_addr_ != MAP_FAILED);

    ehdr_ = *reinterpret_cast<Elf64_Ehdr*>(file_base_addr_);
    for (uint16_t i = 0; i < ehdr_.e_phnum; i++) {
        Elf64_Phdr ph = *reinterpret_cast<Elf64_Phdr*>(
            file_base_addr_ + ehdr_.e_phoff + i * ehdr_.e_phentsize);
        file_phdrs_.emplace_back(ph);

        if (ph.p_type == PT_DYNAMIC) {
            LOG(INFO) << "Found PT_DYNAMIC";
            file_dynamic_ = ph;
        }
    }
}

void ELFBinary::Load(Elf64_Addr base_addr) {
    base_addr_ = base_addr;
    end_addr_ = base_addr_;
    LOG(INFO) << "Load start " << path_;
    for (auto ph : file_phdrs_) {
        if (ph.p_type != PT_LOAD) {
            continue;
        }
        LOG(INFO) << LOG_BITS(reinterpret_cast<void*>(ph.p_vaddr))
                  << LOG_BITS(ph.p_memsz);
        void* mmap_start =
            reinterpret_cast<void*>((ph.p_vaddr + base_addr) & (~(0xfff)));
        void* mmap_end = reinterpret_cast<void*>(
            ((ph.p_vaddr + ph.p_memsz + base_addr) + 0xfff) & (~(0xfff)));
        end_addr_ = reinterpret_cast<Elf64_Addr>(mmap_end);
        size_t mmap_size = reinterpret_cast<size_t>(mmap_end) -
                           reinterpret_cast<size_t>(mmap_start);
        char* p = reinterpret_cast<char*>(
            mmap(mmap_start, mmap_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0));
        LOG(INFO) << "mmap: " << LOG_KEY(path_) << LOG_BITS(p)
                  << LOG_BITS(ph.p_vaddr);
        CHECK_EQ(mmap_start, p);
        CHECK_LE(reinterpret_cast<Elf64_Addr>(mmap_start),
                 ph.p_vaddr + base_addr);
        CHECK_LE(ph.p_vaddr + base_addr + ph.p_memsz,
                 reinterpret_cast<Elf64_Addr>(mmap_end));
        LOG(INFO) << LOG_BITS(mmap_start)
                  << LOG_BITS(file_base_addr_ + ph.p_offset)
                  << LOG_BITS(ph.p_filesz);
        memcpy(reinterpret_cast<void*>(ph.p_vaddr + base_addr),
               file_base_addr_ + ph.p_offset, ph.p_filesz);
    }
    LOG(INFO) << "Load end";

    LOG(INFO) << "ParseDynamic start";
    ParseDynamic();
    LOG(INFO) << "ParseDynamic end";
}

void ELFBinary::ParseDynamic() {
    // Must mmap PT_LOADs before call ParseDynamic.
    CHECK_NE(base_addr_, 0);

    const size_t dyn_size = sizeof(Elf64_Dyn);
    CHECK_EQ(file_dynamic_.p_filesz % dyn_size, 0);

    // Search DT_STRTAB at first.
    for (size_t i = 0; i < file_dynamic_.p_filesz / dyn_size; ++i) {
        Elf64_Dyn* dyn = reinterpret_cast<Elf64_Dyn*>(
            base_addr_ + file_dynamic_.p_vaddr + dyn_size * i);
        if (dyn->d_tag == DT_STRTAB) {
            LOG(INFO) << "Found DT_STRTAB";
            strtab_ = reinterpret_cast<char*>(dyn->d_un.d_ptr + base_addr_);
        }
    }

    CHECK_NE(strtab_, nullptr);

    for (size_t i = 0; i < file_dynamic_.p_filesz / dyn_size; ++i) {
        Elf64_Dyn* dyn = reinterpret_cast<Elf64_Dyn*>(
            base_addr_ + file_dynamic_.p_vaddr + dyn_size * i);
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
        }
    }

    LOG(INFO) << LOG_KEY(relasz_) << LOG_KEY(relaent_) << LOG_KEY(relacount_);
    if (rela_ != nullptr) {
        CHECK_EQ(relasz_ % relaent_, 0);
        Elf64_Rela* r = rela_;
        for (int i = 0; i < relasz_ / relaent_; i++, r++) {
            relas_.emplace_back(*r);
        }
    }
}

DynLoader::DynLoader(const std::filesystem::path& main_path)
    : main_path_(main_path) {
    Elf64_Addr base_addr = 0x400000;
    binaries_.emplace_back(ELFBinary(main_path));
    binaries_.back().Load(base_addr);
    base_addr =
        (binaries_.back().end_addr() + (0x400000 - 1)) / 0x400000 * 0x400000;

    std::queue<std::tuple<std::string, std::optional<std::filesystem::path>,
                          std::optional<std::filesystem::path>>>
        queue;
    std::set<std::string> loaded;

    for (const auto& n : binaries_.back().neededs()) {
        queue.push(std::make_tuple(n, binaries_.back().runpath(),
                                   binaries_.back().rpath()));
    }

    while (!queue.empty()) {
        const auto [library_name, runpath, rpath] = queue.front();
        queue.pop();

        if (loaded.count(library_name) != 0) continue;
        loaded.insert(library_name);
        if (library_name.find("ld-linux") != std::string::npos) {
            LOG(INFO) << "Skip " << library_name;
            continue;
        }

        const auto library_path = FindLibrary(library_name, runpath, rpath);
        binaries_.emplace_back(ELFBinary(library_path));
        binaries_.back().Load(base_addr);
        base_addr = (binaries_.back().end_addr() + (0x400000 - 1)) / 0x400000 *
                    0x400000;
        for (const auto& n : binaries_.back().neededs()) {
            queue.push(std::make_tuple(n, binaries_.back().runpath(),
                                       binaries_.back().rpath()));
        }
    }
}

std::filesystem::path DynLoader::FindLibrary(
    std::string library_name, std::optional<std::filesystem::path> runpath,
    std::optional<std::filesystem::path> rpath) {
    std::vector<std::filesystem::path> library_directory;
    if (runpath) {
        library_directory.emplace_back(runpath.value());
    }
    if (rpath) {
        library_directory.emplace_back(rpath.value());
    }
    const auto ldsoconfs = read_ldsoconf();
    library_directory.insert(library_directory.end(), ldsoconfs.begin(),
                             ldsoconfs.end());
    library_directory.emplace_back("/lib");
    library_directory.emplace_back("/usr/lib");
    library_directory.emplace_back("/usr/lib64");

    for (const auto& d : library_directory) {
        std::filesystem::path p = d / library_name;
        if (std::filesystem::exists(p)) {
            return p;
        }
    }
    LOG(FATAL) << "Cannot find " << LOG_KEY(library_name);
    std::abort();
}

std::unique_ptr<DynLoader> MakeDynLoader(
    const std::filesystem::path& main_path) {
    return std::make_unique<DynLoader>(main_path);
}
