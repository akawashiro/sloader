#include <dlfcn.h>

int main(){
    void* handle = dlopen("libdlopen_hoge.so", RTLD_NOW);
    void (*hoge_func)();
    *(void**)(&hoge_func) = dlsym(handle, "hoge");
    hoge_func();
}
