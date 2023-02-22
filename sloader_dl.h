#include <dlfcn.h>

void* sloader_dlopen(const char* filename, int flags);
void* sloader_dlsym(void* handle, const char* symbol);
void* sloader_dlvsym(void* handle, char* symbol, char* version);
int sloader_dladdr(void *addr, Dl_info *info);
