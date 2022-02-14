#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define LOG() std::cerr << __FILE__ << ":" << __LINE__ << " "
#define LOG_KEY_VALUE(key, value) key << "=" << value << " "
#define LOG_KEY(key) LOG_KEY_VALUE(#key, key)
#define LOG_BITS(key) LOG_KEY_VALUE(#key, HexString(key))

#define CHECK(cond)                  \
    if (!(cond)) {                   \
        LOG() << #cond << std::endl; \
        std::abort();                \
    }
#define CHECK_EQ(a, b)                                                    \
    if ((a) != (b)) {                                                     \
        LOG() << #a << "(" << HexString(a) << ")"                         \
              << " != " << #b << "(" << HexString(b) << ")" << std::endl; \
        std::abort();                                                     \
    }
#define CHECK_NE(a, b)                            \
    if ((a) == (b)) {                             \
        LOG() << #a << " == " << #b << std::endl; \
        std::abort();                             \
    }
#define CHECK_GT(a, b)                            \
    if ((a) <= (b)) {                             \
        LOG() << #a << " <= " << #b << std::endl; \
        std::abort();                             \
    }
#define CHECK_LE(a, b)                           \
    if ((a) > (b)) {                             \
        LOG() << #a << " > " << #b << std::endl; \
        std::abort();                            \
    }
#define CHECK_LT(a, b)                           \
    if ((a) >= (b)) {                            \
        LOG() << #a << " > " << #b << std::endl; \
        std::abort();                            \
    }

std::string HexString(char* num, int length = -1) {
    if (length == -1) {
        length = 16;
    }
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(length)
       << std::hex << (u_int64_t)num;
    return ss.str();
}

std::string HexString(const char* num, int length = -1) {
    if (length == -1) {
        length = 16;
    }
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(length)
       << std::hex << (u_int64_t)num;
    return ss.str();
}

template <class T>
std::string HexString(T num, int length = -1) {
    if (length == -1) {
        length = sizeof(T) * 2;
    }
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(length)
       << std::hex << +num;
    return ss.str();
}

class ELF {
   public:
    ELF(const std::string& filename, char* head, const size_t size)
        : filename_(filename), head_(head), size_(size) {
        ehdr_ = reinterpret_cast<Elf64_Ehdr*>(head);
        CHECK_EQ(ehdr()->e_type, ET_EXEC);
        for (uint16_t i = 0; i < ehdr()->e_phnum; i++) {
            phdrs_.emplace_back(reinterpret_cast<Elf64_Phdr*>(
                head_ + ehdr()->e_phoff + i * ehdr()->e_phentsize));
        }
        for (auto ph : phdrs()) {
            LOG() << LOG_BITS(ph->p_type) << std::endl;
            if (ph->p_type == PT_DYNAMIC) {
                CHECK(ph_dynamic_ == NULL);
                ph_dynamic_ = ph;
            }
        }
    }

    void Show() {
        LOG() << "Ehdr:" << LOG_BITS(ehdr()->e_entry) << LOG_BITS(size_)
              << LOG_BITS(head_) << std::endl;
        for (auto p : phdrs()) {
            LOG() << "Phdr:" << LOG_BITS(p->p_vaddr) << LOG_BITS(p->p_offset)
                  << LOG_BITS(p->p_filesz) << std::endl;
        }
    }

    void Load() {
        LOG() << "Load start" << std::endl;
        for (auto ph : phdrs()) {
            if (ph->p_type != PT_LOAD) {
                continue;
            }
            LOG() << LOG_BITS(reinterpret_cast<void*>(ph->p_vaddr))
                  << LOG_BITS(ph->p_memsz) << std::endl;
            void* mmap_start =
                reinterpret_cast<void*>((ph->p_vaddr) & (~(0xfff)));
            void* mmap_end = reinterpret_cast<void*>(
                ((ph->p_vaddr + ph->p_memsz) + 0xfff) & (~(0xfff)));
            size_t mmap_size = reinterpret_cast<size_t>(mmap_end) -
                               reinterpret_cast<size_t>(mmap_start);
            char* p = reinterpret_cast<char*>(
                mmap(mmap_start, mmap_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0));
            LOG() << "mmap: " << LOG_KEY(filename()) << LOG_BITS(p)
                  << LOG_BITS(ph->p_vaddr) << std::endl;
            CHECK_EQ(mmap_start, p);
            CHECK_LE(reinterpret_cast<Elf64_Addr>(mmap_start), ph->p_vaddr);
            CHECK_LT(ph->p_vaddr + ph->p_memsz,
                     reinterpret_cast<Elf64_Addr>(mmap_end));
            LOG() << LOG_BITS(p) << LOG_BITS(head() + ph->p_offset)
                  << LOG_BITS(ph->p_filesz) << std::endl;
            memcpy(reinterpret_cast<void*>(ph->p_vaddr), head() + ph->p_offset,
                   ph->p_filesz);
            memories_.emplace_back(std::make_pair(p, ph->p_memsz));
        }
        LOG() << "Load end" << std::endl;
    }

    void Unload() {
        for (auto p : memories_) {
            munmap(p.first, p.second);
        }
    }

    void Relocate() {}

    // To variables of stack and stack_num assign to %rdi and %rsi, I use the
    // calling convention. For details, see A.2.1 Calling Conventions in
    // https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf. Off course,
    // compiler must not inline this function.
    void __attribute__((noinline))
    ExecuteCore(uint64_t* stack, size_t stack_num) {
        for (int i = 0; i < stack_num; i++) {
            asm volatile("pushq %0" ::"m"(*(stack + i)));
        }

        asm volatile("jmp *%0" ::"m"(ehdr()->e_entry));
    }

    void Execute(std::vector<std::string> envs) {
        unsigned long at_random = getauxval(AT_RANDOM);
        unsigned long at_pagesz = getauxval(AT_PAGESZ);
        CHECK_NE(at_random, 0);
        LOG() << LOG_BITS(at_random) << LOG_BITS(at_pagesz) << std::endl;

        // Some commented out auxiliary values because they are not appropriate
        // as loading programs. These values are for sloader itself.
        std::vector<unsigned long> aux_types{
            AT_IGNORE,
            // AT_EXECFD,
            // AT_PHDR,
            AT_PHENT,
            // AT_PHNUM,
            AT_PAGESZ,
            // AT_BASE,
            AT_FLAGS,
            // AT_ENTRY,
            AT_NOTELF, AT_UID, AT_EUID, AT_GID, AT_EGID, AT_CLKTCK, AT_PLATFORM,
            AT_HWCAP, AT_FPUCW, AT_DCACHEBSIZE, AT_ICACHEBSIZE, AT_UCACHEBSIZE,
            AT_IGNOREPPC, AT_SECURE, AT_BASE_PLATFORM, AT_RANDOM, AT_HWCAP2,
            // AT_EXECFN,
            AT_SYSINFO, AT_SYSINFO_EHDR, AT_L1I_CACHESHAPE, AT_L1D_CACHESHAPE,
            AT_L2_CACHESHAPE, AT_L3_CACHESHAPE, AT_L1I_CACHESIZE,
            AT_L1I_CACHEGEOMETRY, AT_L1D_CACHESIZE, AT_L1D_CACHEGEOMETRY,
            AT_L2_CACHESIZE, AT_L2_CACHEGEOMETRY, AT_L3_CACHESIZE,
            AT_L3_CACHEGEOMETRY, AT_MINSIGSTKSZ};

        std::vector<std::pair<unsigned long, unsigned long>> aux_tvs;
        for (int i = 0; i < aux_types.size(); i++) {
            unsigned long v = getauxval(aux_types[i]);
            if (v != 0) {
                aux_tvs.emplace_back(std::make_pair(aux_types[i], v));
                LOG() << LOG_BITS(aux_types[i]) << LOG_BITS(v) << std::endl;
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
        size_t stack_num = 4 + 2 + 2 * aux_tvs.size() + 1 + envs.size() + 3;
        size_t stack_size = sizeof(uint64_t) * stack_num;
        unsigned long* stack = reinterpret_cast<uint64_t*>(malloc(stack_size));
        memset(stack, 0, stack_size);

        // 4 words padding
        stack_index += 4;

        // First two elements are 0 and AT_NULL.
        stack_index += 2;

        // auxs
        for (int i = 0; i < aux_tvs.size(); i++) {
            *(stack + stack_index) = aux_tvs[i].second;
            stack_index++;
            *(stack + stack_index) = aux_tvs[i].first;
            stack_index++;
        }

        // End of environment variables
        stack_index++;

        // Environment variables
        for (int i = 0; i < envs.size(); i++) {
            *(stack + stack_index) =
                reinterpret_cast<uint64_t>(envs[i].c_str());
            stack_index++;
        }

        // argv[argc]
        stack_index++;

        // argv[0]
        *(stack + stack_index) = reinterpret_cast<uint64_t>(filename().c_str());
        stack_index++;

        // argc
        *(stack + stack_index) = 1;
        stack_index++;

        CHECK_EQ(stack_index, stack_num);

        ExecuteCore(stack, stack_num);

        free(stack);
        LOG() << "Execute end" << std::endl;
    }
    std::string filename() { return filename_; }
    Elf64_Ehdr* ehdr() { return ehdr_; }
    char* head() { return head_; }
    std::vector<Elf64_Phdr*> phdrs() { return phdrs_; }

   private:
    char* head_;
    size_t size_;
    std::string filename_;
    Elf64_Ehdr* ehdr_;
    std::vector<Elf64_Phdr*> phdrs_;
    Elf64_Phdr* ph_dynamic_ = NULL;
    std::vector<Elf64_Dyn*> dyns_;
    std::vector<std::pair<char*, uint32_t>> memories_;
};

std::unique_ptr<ELF> ReadELF(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    CHECK(fd >= 0);

    size_t size = lseek(fd, 0, SEEK_END);
    CHECK_GT(size, 8 + 16);

    size_t mapped_size = (size + 0xfff) & ~0xfff;

    char* p = (char*)mmap(NULL, mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE, fd, 0);
    CHECK(p != MAP_FAILED);

    return std::make_unique<ELF>(filename, p, mapped_size);
}

void ShowHelp(std::ostream& os) {
    os << "sloader -- Simple ELF loader" << std::endl;
}

int main(int argc, char* const argv[], char** envp) {
    static option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {0, 0, 0, 0},
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                ShowHelp(std::cout);
                return 0;
                break;
            default:
                LOG() << "Found unsupported option" << std::endl;
                CHECK(false);
                break;
        }
    }

    CHECK_LT(optind, argc);

    std::string file = argv[optind++];

    std::vector<std::string> envs;
    for (char** env = envp; *env != 0; env++) {
        envs.emplace_back(*env);
    }

    auto main_binary = ReadELF(file);
    main_binary->Show();
    main_binary->Load();
    main_binary->Execute(envs);
    main_binary->Unload();
}
