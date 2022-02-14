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

    void Execute(std::vector<std::string> envs) {
        unsigned long at_random = getauxval(AT_RANDOM);
        unsigned long at_pagesz = getauxval(AT_PAGESZ);
        CHECK_NE(at_random, 0);
        LOG() << LOG_BITS(at_random) << LOG_BITS(at_pagesz) << std::endl;

        LOG() << "Execute start " << LOG_KEY(filename()) << std::endl;
        const char* argv0 = filename().c_str();

        const size_t envsize = envs.size();
        const char** envps =
            reinterpret_cast<const char**>(malloc(sizeof(char*) * envsize));
        for (int i = 0; i < envs.size(); i++) {
            *(envps + i) = envs[i].c_str();
        }

        // See http://articles.manugarg.com/aboutelfauxiliaryvectors.html for
        // the stack layout padding.
        asm volatile("push $0");  // 0
        asm volatile("push $0");  // 0
        asm volatile("push $0");  // 0
        asm volatile("push $0");  // 0

        // Auxiliary vectors
        // TODO(akawashiro): Can we use constants such as AT_RANDOM in assembly?
        asm volatile("push $0"); 
        asm volatile("push $0");  // AT_NULL
        asm volatile("push %0" ::"r"(at_random));
        asm volatile("push $25");  // AT_RANDOM
        asm volatile("push %0" ::"r"(at_pagesz));
        asm volatile("push $6");  // AT_PAGESZ
        asm volatile("push %0" ::"m"(argv0));
        asm volatile("push $31");  // AT_EXECFN

        // Environment variables
        asm volatile("push $0");
        for (int i = 0; i < envsize; i++) {
            asm volatile("push %0" ::"m"(*(envps + i)));
        }

        // Argument from user
        asm volatile("push $0");               // argv[argc] (must be NULL)
        asm volatile("push %0" ::"m"(argv0));  // argv[0]
        asm volatile("push $1");               // argc

        asm volatile("jmp *%0" ::"m"(ehdr()->e_entry));
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

void ShowHelp(std::ostream& os){
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
