#include <dlfcn.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <glob.h>

#include "sloader_dl.h"
#include "dyn_loader.h"
#include "utils.h"

void* sloader_dlopen(const char* filename, int flags) {
    auto b = ELFBinary(filename);
    LOG(INFO) << LOG_KEY(b.filename());
    printf("sloader_dlopen: filename=%s flags=%d\n", filename, flags);
    void* r = dlopen(filename, flags);
    if (r == NULL) {
        printf("sloader_dlopen failed\n");
    }
    printf("r=%lu errno=%d\n", reinterpret_cast<unsigned long>(r), errno);
    return r;
}

void* sloader_dlsym(void* handle, const char* symbol) {
    printf("sloader_dlsym: handle=%p symbol=%s\n", handle, symbol);
    return dlsym(handle, symbol);
}

void* sloader_dlvsym(void* handle, char* symbol, char* version) {
    printf("sloader_dlvsym: handle=%p symbol=%s version=%s\n", handle, symbol, version);
    exit(10);
    return dlvsym(handle, symbol, version);
}
