#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <glog/logging.h>
#include <sys/mman.h>

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#define LOG_KEY_VALUE(key, value) " " << key << "=" << value
#define LOG_KEY(key) LOG_KEY_VALUE(#key, key)
#define LOG_BITS(key) LOG_KEY_VALUE(#key, HexString(key))

template <class T>
inline std::string HexString(T num, int length = -1) {
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
    }
    void Show() {
        std::cout << LOG_BITS(ehdr()->e_entry) << LOG_BITS(size_)
                  << LOG_BITS(reinterpret_cast<const int*>(head_)) << std::endl;
    }
    void ExecuteEntry() {
        const void* p = head_ + ehdr()->e_entry;
        void (*fp)(void) = ((init_t)(ehdr()->e_entry));
        fp();
    }
    const std::string filename() { return filename_; }
    const Elf64_Ehdr* ehdr() { return ehdr_; }

   private:
    const char* head_;
    const size_t size_;
    const std::string filename_;
    Elf64_Ehdr* ehdr_;
};

std::unique_ptr<ELF> ReadELF(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    CHECK(fd >= 0) << "Failed to open " << filename;

    size_t size = lseek(fd, 0, SEEK_END);
    CHECK_GT(size, 8 + 16) << "Too small file: " << filename;

    size_t mapped_size = (size + 0xfff) & ~0xfff;

    char* p =
        (char*)mmap(reinterpret_cast<void*>(0x400000), mapped_size,
                    PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
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
    main_binary->ExecuteEntry();
}
