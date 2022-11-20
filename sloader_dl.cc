#include <dlfcn.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <elf.h>
#include <glob.h>

#include "dyn_loader.h"
#include "sloader_dl.h"
#include "utils.h"

void* sloader_dlopen(const char* filename, int flags) {
    LOG(INFO) << "sloader_dlopen" << LOG_KEY(filename) << LOG_KEY(flags);
    GetDynLoader()->LoadDependingLibs(filename);
    GetDynLoader()->Relocate();
    // TODO: Functions depending on handle may fail.
    return reinterpret_cast<void*>(0xdeadbeefdeadbeef);
}

void* sloader_dlsym(void* handle, const char* symbol) {
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
