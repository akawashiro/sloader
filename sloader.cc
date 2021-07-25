#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <glog/logging.h>
#include <sys/mman.h>

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define LOG_KEY_VALUE(key, value) " " << key << "=" << value
#define LOG_KEY(key) LOG_KEY_VALUE(#key, key)
#define LOG_BITS(key) LOG_KEY_VALUE(#key, HexString(key))

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

typedef void (*init_t)();

class ELF {
   public:
    ELF(const std::string& filename, char* head, const size_t size)
        : filename_(filename), head_(head), size_(size) {
        ehdr_ = reinterpret_cast<Elf64_Ehdr*>(head);
        CHECK_EQ(ehdr()->e_type, ET_EXEC);
        for (uint16_t i = 0; i < ehdr()->e_phnum; i++) {
            phdrs_.emplace_back(reinterpret_cast<const Elf64_Phdr*>(
                head_ + ehdr()->e_phoff + i * ehdr()->e_phentsize));
        }
    }
    void Show() {
        std::cout << "Ehdr:" << LOG_BITS(ehdr()->e_entry) << LOG_BITS(size_)
                  << LOG_BITS(head_) << std::endl;
        for (auto p : phdrs()) {
            std::cout << "Phdr:" << LOG_BITS(p->p_vaddr)
                      << LOG_BITS(p->p_offset) << LOG_BITS(p->p_filesz)
                      << std::endl;
        }
    }
    void Load() {
        for (auto ph : phdrs()) {
            std::cout << LOG_BITS(reinterpret_cast<void*>(ph->p_vaddr))
                      << LOG_BITS(ph->p_memsz) << std::endl;
            char* p = reinterpret_cast<char*>(
                mmap(reinterpret_cast<void*>(ph->p_vaddr), ph->p_memsz,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0));
            CHECK(p != MAP_FAILED) << "mmap failed: " << filename();
            std::cout << LOG_BITS(p) << LOG_BITS(head() + ph->p_offset)
                      << LOG_BITS(ph->p_filesz) << std::endl;
            memcpy(p, head() + ph->p_offset, ph->p_filesz);
            memories_.emplace_back(std::make_pair(p, ph->p_memsz));
        }
    }
    void Unload() {
        for (auto p : memories_) {
            munmap(p.first, p.second);
        }
    }
    void Execute() {
        const void* p = reinterpret_cast<const void*>(ehdr()->e_entry);
        void (*fp)(void) = ((init_t)(ehdr()->e_entry));
        fp();
    }
    const std::string filename() { return filename_; }
    const Elf64_Ehdr* ehdr() { return ehdr_; }
    const char* head() { return head_; }
    const std::vector<const Elf64_Phdr*> phdrs() { return phdrs_; }

   private:
    const char* head_;
    const size_t size_;
    const std::string filename_;
    Elf64_Ehdr* ehdr_;
    std::vector<const Elf64_Phdr*> phdrs_;
    std::vector<std::pair<char*, uint32_t>> memories_;
};

std::unique_ptr<ELF> ReadELF(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    CHECK(fd >= 0) << "Failed to open " << filename;

    size_t size = lseek(fd, 0, SEEK_END);
    CHECK_GT(size, 8 + 16) << "Too small file: " << filename;

    size_t mapped_size = (size + 0xfff) & ~0xfff;

    char* p = (char*)mmap(NULL, mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE, fd, 0);
    CHECK(p != MAP_FAILED) << "mmap failed: " << filename;

    return std::make_unique<ELF>(filename, p, mapped_size);
}

int main(int argc, char* const argv[]) {
    google::InitGoogleLogging(argv[0]);

    static option long_options[] = {
        {"load", required_argument, nullptr, 'l'},
        {0, 0, 0, 0},
    };

    std::string file;

    int opt;
    while ((opt = getopt_long(argc, argv, "l:", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'l':
                file = optarg;
                break;
            default:
                CHECK(false);
                break;
        }
    }

    if (optind < argc) {
        CHECK(false) << "Fail to parse arguments.";
    }

    auto main_binary = ReadELF(file);
    main_binary->Show();
    main_binary->Load();
    main_binary->Execute();
    main_binary->Unload();
}
