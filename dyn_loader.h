#pragma once

#include <elf.h>
#include <glob.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <tuple>

#include "utils.h"

class ELFBinary {
public:
    ELFBinary(const std::filesystem::path path);
    ELFBinary(const ELFBinary&) = default;

    const std::string filename() const { return path_.filename().string(); }
    const Elf64_Addr GetSymbolAddr(const size_t symbol_index);
    const std::vector<std::string> neededs() const { return neededs_; }
    const std::vector<Elf64_Sym>& symtabs() const { return symtabs_; }
    const std::optional<std::filesystem::path> runpath() const { return runpath_; }
    const std::optional<std::filesystem::path> rpath() const { return rpath_; }
    const bool has_tls() const { return has_tls_; }
    const Elf64_Phdr file_tls() const { return file_tls_; }
    const Elf64_Addr base_addr() const { return base_addr_; }
    const Elf64_Addr end_addr() const { return end_addr_; }
    const std::filesystem::path path() const { return path_; }
    const std::vector<Elf64_Rela> relas() const { return relas_; }
    const std::vector<Elf64_Rela> pltrelas() const { return pltrelas_; }
    const Elf64_Xword init() const { return init_; }
    const Elf64_Xword fini() const { return fini_; }
    const Elf64_Xword init_arraysz() const { return init_arraysz_; }
    const Elf64_Xword init_array() const { return init_array_; }
    const Elf64_Xword fini_arraysz() const { return fini_arraysz_; }
    const Elf64_Xword fini_array() const { return fini_array_; }
    const char* strtab() const { return strtab_; }
    const Elf64_Ehdr ehdr() const { return ehdr_; }

    Elf64_Addr Load(Elf64_Addr base_addr, std::shared_ptr<std::ofstream> map_file);
    void ParseDynamic();

private:
    // To generate copy constructor, we cannot make member variables const.
    std::filesystem::path path_;
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
    Elf64_Xword init_ = 0;
    Elf64_Xword fini_ = 0;
    Elf64_Xword init_arraysz_ = 0;
    Elf64_Xword init_array_ = 0;
    Elf64_Xword fini_arraysz_ = 0;
    Elf64_Xword fini_array_ = 0;
    Elf64_Sym* symtab_ = nullptr;
    std::vector<Elf64_Sym> symtabs_;
    Elf64_Xword syment_ = 0;
    std::vector<Elf64_Phdr> file_phdrs_;
    Elf64_Phdr file_dynamic_ = {.p_filesz = 0};
    bool has_tls_ = false;
    Elf64_Phdr file_tls_;
    std::vector<std::string> neededs_;
    std::optional<std::filesystem::path> runpath_ = std::nullopt;
    std::optional<std::filesystem::path> rpath_ = std::nullopt;
    char* strtab_ = nullptr;
    Elf64_Xword strsz_ = 0;
};

class DynLoader {
public:
    DynLoader(const std::filesystem::path& main_path, const std::vector<std::string>& args, const std::vector<std::string>& envs);
    // The main function
    void Run();

    void LoadDependingLibs(const std::filesystem::path& root_path);
    void Relocate();
    std::optional<std::pair<size_t, size_t>> SearchSym(const std::string& name, bool skip_main);
    std::vector<ELFBinary> binaries_;

private:
    std::filesystem::path main_path_;
    std::shared_ptr<std::ofstream> map_file_;
    const std::vector<std::string> args_;
    const std::vector<std::string> envs_;
    Elf64_Addr next_base_addr_;
    std::set<std::string> loaded_;
    std::map<std::filesystem::path, bool> relocated_;

    void Execute(std::vector<std::string> args, std::vector<std::string> envs);
    void __attribute__((noinline)) ExecuteCore(uint64_t* stack, size_t stack_num, uint64_t entry);
    Elf64_Addr TLSSymOffset(const std::string& name);
};

void InitializeDynLoader(const std::filesystem::path& main_path, const std::vector<std::string>& envs,
                         const std::vector<std::string>& argv);

std::shared_ptr<DynLoader> GetDynLoader();

std::filesystem::path FindLibrary(std::string library_name, std::optional<std::filesystem::path> runpath,
                                  std::optional<std::filesystem::path> rpath);
