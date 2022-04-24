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

    void Load(Elf64_Addr base_addr) {
        base_addr_ = base_addr;
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

    void ParseDynamic() {
        const size_t dyn_size = sizeof(Elf64_Dyn);
        CHECK_EQ(file_dynamic_.p_filesz % dyn_size, 0);

        for (size_t i = 0; i < file_dynamic_.p_filesz / dyn_size; ++i) {
            LOG(INFO) << LOG_KEY(i);
            Elf64_Dyn* dyn = reinterpret_cast<Elf64_Dyn*>(
                base_addr_ + file_dynamic_.p_offset + dyn_size * i);
            if (dyn->d_tag == DT_STRTAB) {
                LOG(INFO) << "Found DT_STRTAB";
                strtab_ = reinterpret_cast<char*>(dyn->d_un.d_ptr + base_addr_);
            }
        }

        CHECK_NE(strtab_, nullptr);

        for (size_t i = 0; i < file_dynamic_.p_filesz / dyn_size; ++i) {
            Elf64_Dyn* dyn = reinterpret_cast<Elf64_Dyn*>(
                base_addr_ + file_dynamic_.p_offset + dyn_size * i);
            if (dyn->d_tag == DT_NEEDED) {
                std::string needed = strtab_ + dyn->d_un.d_val;
                neededs_.emplace_back(needed);
            }
        }
    }

   private:
    const std::filesystem::path path_;
    char* file_base_addr_;
    Elf64_Addr base_addr_ = 0;
    Elf64_Ehdr ehdr_;
    std::vector<Elf64_Phdr> file_phdrs_;
    Elf64_Phdr file_dynamic_;
    std::vector<std::string> neededs_;
    char* strtab_ = nullptr;
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
