#pragma once

#include <elf.h>
#include <glob.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <queue>
#include <set>
#include <tuple>

#include "utils.h"

class ELFBinary {
   public:
    ELFBinary(const std::filesystem::path path);
    void Load(Elf64_Addr base_addr);
    void ParseDynamic();
    std::vector<std::string> neededs() { return neededs_; }
    std::optional<std::filesystem::path> runpath() { return runpath_; }
    std::optional<std::filesystem::path> rpath() { return rpath_; }
    Elf64_Addr end_addr() { return end_addr_; }

   private:
    const std::filesystem::path path_;
    char* file_base_addr_;
    Elf64_Addr base_addr_ = 0;
    Elf64_Addr end_addr_ = 0;
    Elf64_Ehdr ehdr_;
    Elf64_Rela* rela_ = nullptr;
    Elf64_Rela* jmprel_ = nullptr;
    std::vector<Elf64_Rela> relas_;
    std::vector<Elf64_Rela> pltrelas_;
    Elf64_Xword relasz_ = 0;
    Elf64_Xword relacount_ = 0;  // What's this?
    Elf64_Xword relaent_ = 0;
    Elf64_Xword pltrelsz_ = 0;
    Elf64_Xword pltrel_ = 0;
    Elf64_Xword pltrelent_ = 0;
    std::vector<Elf64_Phdr> file_phdrs_;
    Elf64_Phdr file_dynamic_;
    std::vector<std::string> neededs_;
    std::optional<std::filesystem::path> runpath_ = std::nullopt;
    std::optional<std::filesystem::path> rpath_ = std::nullopt;
    char* strtab_ = nullptr;
};

class DynLoader {
   public:
    DynLoader(const std::filesystem::path& main_path);

   private:
    std::filesystem::path FindLibrary(
        std::string library_name, std::optional<std::filesystem::path> runpath,
        std::optional<std::filesystem::path> rpath);
    std::filesystem::path main_path_;
    std::vector<ELFBinary> binaries_;
};

std::unique_ptr<DynLoader> MakeDynLoader(
    const std::filesystem::path& main_path);
