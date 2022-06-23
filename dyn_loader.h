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
    void Load(Elf64_Addr base_addr, std::ofstream& map_file);
    void ParseDynamic();
    const std::string filename() { return path_.filename().string(); }
    const Elf64_Addr GetSymbolAddr(const size_t symbol_index);
    std::vector<std::string> neededs() { return neededs_; }
    const std::vector<Elf64_Sym> symtabs() const { return symtabs_; }
    std::optional<std::filesystem::path> runpath() { return runpath_; }
    std::optional<std::filesystem::path> rpath() { return rpath_; }
    const Elf64_Addr base_addr() const { return base_addr_; }
    const Elf64_Addr end_addr() const { return end_addr_; }
    const std::filesystem::path path() const { return path_; }
    const std::vector<Elf64_Rela> relas() const { return relas_; }
    const std::vector<Elf64_Rela> pltrelas() const { return pltrelas_; }
    const char* strtab() const { return strtab_; }
    const Elf64_Ehdr ehdr() const { return ehdr_; }

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
    Elf64_Sym* symtab_ = nullptr;
    std::vector<Elf64_Sym> symtabs_;
    Elf64_Xword syment_ = 0;
    std::vector<Elf64_Phdr> file_phdrs_;
    Elf64_Phdr file_dynamic_;
    std::vector<std::string> neededs_;
    std::optional<std::filesystem::path> runpath_ = std::nullopt;
    std::optional<std::filesystem::path> rpath_ = std::nullopt;
    char* strtab_ = nullptr;
    Elf64_Xword strsz_ = 0;
};

class DynLoader {
   public:
    DynLoader(const std::filesystem::path& main_path,
              const std::vector<std::string>& envs);
    void Execute(std::vector<std::string> envs);

   private:
    void __attribute__((noinline))
    ExecuteCore(uint64_t* stack, size_t stack_num, uint64_t entry);
    std::filesystem::path FindLibrary(
        std::string library_name, std::optional<std::filesystem::path> runpath,
        std::optional<std::filesystem::path> rpath);
    std::filesystem::path main_path_;
    const std::vector<std::string> envs_;
    std::vector<ELFBinary> binaries_;
    void Relocate();
    std::optional<std::pair<size_t, size_t>> SearchSym(const std::string& name,
                                                       bool skip_main);
};

std::unique_ptr<DynLoader> MakeDynLoader(const std::filesystem::path& main_path,
                                         const std::vector<std::string>& envs);
