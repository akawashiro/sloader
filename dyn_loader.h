#pragma once

#include "utils.h"

class ELFBinary {
   public:
    ELFBinary(const std::filesystem::path path) : path_(path) {
        int fd = open(path_.c_str(), O_RDONLY);
        CHECK(fd >= 0);

        size_t size = lseek(fd, 0, SEEK_END);
        CHECK_GT(size, 8 + 16);

        size_t mapped_size = (size + 0xfff) & ~0xfff;

        head_ =
            (char*)mmap(NULL, mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE, fd, 0);
        CHECK(head_ != MAP_FAILED);

        ehdr_ = *reinterpret_cast<Elf64_Ehdr*>(head_);
        for (uint16_t i = 0; i < ehdr_.e_phnum; i++) {
            phdrs_.emplace_back(*reinterpret_cast<Elf64_Phdr*>(
                head_ + ehdr_.e_phoff + i * ehdr_.e_phentsize));
        }
    }

    void Load(Elf64_Addr base_addr) {
        LOG(INFO) << "Load start " << path_;
        for (auto ph : phdrs_) {
            if (ph.p_type != PT_LOAD) {
                continue;
            }
            LOG(INFO) << LOG_BITS(reinterpret_cast<void*>(ph.p_vaddr))
                      << LOG_BITS(ph.p_memsz);
            void* mmap_start =
                reinterpret_cast<void*>((ph.p_vaddr + base_addr) & (~(0xfff)));
            void* mmap_end = reinterpret_cast<void*>(
                ((ph.p_vaddr + ph.p_memsz + base_addr) + 0xfff) & (~(0xfff)));
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
            CHECK_LT(ph.p_vaddr + base_addr + ph.p_memsz,
                     reinterpret_cast<Elf64_Addr>(mmap_end));
            LOG(INFO) << LOG_BITS(mmap_start) << LOG_BITS(head_ + ph.p_offset)
                      << LOG_BITS(ph.p_filesz);
            memcpy(mmap_start, head_ + ph.p_offset, ph.p_filesz);
        }
        LOG(INFO) << "Load end";
    }

   private:
    const std::filesystem::path path_;
    char* head_;
    Elf64_Ehdr ehdr_;
    std::vector<Elf64_Phdr> phdrs_;
};

class DynLoader {
   public:
    DynLoader(const std::filesystem::path& main_path) : main_path_(main_path) {
        binaries_.emplace_back(ELFBinary(main_path));
        binaries_[0].Load(0x400000);
    }

   private:
    std::filesystem::path main_path_;
    std::vector<ELFBinary> binaries_;
};

std::unique_ptr<DynLoader> MakeDynLoader(
    const std::filesystem::path& main_path) {
    return std::make_unique<DynLoader>(main_path);
}
