#include <dlfcn.h>
#include <stdio.h>

void fromMain(){
    printf("From main\n");
}

int main(){
    void* handle = dlopen("libdlopen2_hoge.so", RTLD_NOW);
    if(handle == NULL) return 1;
    void (*hoge_func)();
    *(void**)(&hoge_func) = dlsym(handle, "hoge");
    hoge_func();
}
