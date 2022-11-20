void* sloader_dlopen(const char* filename, int flags);
void* sloader_dlsym(void* handle, const char* symbol);
void* sloader_dlvsym(void* handle, char* symbol, char* version);
