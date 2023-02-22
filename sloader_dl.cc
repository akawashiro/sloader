#include <dlfcn.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <optional>

#include <elf.h>
#include <glob.h>

#include "dyn_loader.h"
#include "sloader_dl.h"
#include "utils.h"

void* sloader_dlopen(const char* filename, int flags) {
    // If filename is NULL, then the returned handle is for the main program. Ref
    // dlopen(3).
    if (filename == NULL) {
        return reinterpret_cast<void*>(0xabcdabcdabcdabcd);
    }
    // Skip dynamic loader and libc.so
    {
        std::string filename_str = std::string(filename);
        if (filename_str.find("ld-linux") != std::string::npos || filename_str.find("libc.so") != std::string::npos) {
            LOG(INFO) << "Skip " << filename_str;
            return reinterpret_cast<void*>(0xaaaaaaaabbbbbbbb);
        }
    }

    LOG(INFO) << "sloader_dlopen" << LOG_KEY(filename) << LOG_KEY(flags) << LOG_KEY(getenv("LD_LIBRARY_PATH"));
    GetDynLoader()->LoadDependingLibs(FindLibrary(filename, std::nullopt, std::nullopt));
    GetDynLoader()->Relocate();
    // TODO: Functions depending on handle may fail.
    return reinterpret_cast<void*>(0xdeadbeefdeadbeef);
}

void* sloader_dlsym(void* handle, const char* symbol) {
    LOG(INFO) << "sloader_dlopen" << LOG_KEY(symbol);
    const auto opt = GetDynLoader()->SearchSym(symbol, false);
    CHECK(opt);
    const auto [bin_index, sym_index] = opt.value();
    Elf64_Addr sym_addr = GetDynLoader()->binaries_[bin_index].GetSymbolAddr(sym_index);
    return reinterpret_cast<void*>(sym_addr);
}

void* sloader_dlvsym(void* handle, char* symbol, char* version) {
    printf("sloader_dlvsym: handle=%p symbol=%s version=%s\n", handle, symbol, version);
    exit(10);
    return dlvsym(handle, symbol, version);
}

int sloader_dladdr(void* addr, Dl_info* info) {
    LOG(INFO) << "sloader_dladdr";
    return dladdr(addr, info);
}
