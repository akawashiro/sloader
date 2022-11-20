#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void fromMain(){
    printf("From main\n");
}

int main(){
    void* handle = dlopen("./libdlopen2_hoge.so", RTLD_NOW);
    if(handle == NULL){
        printf("handle is NULL\n");
        exit(1);
    }
    void (*hoge_func)();
    *(void**)(&hoge_func) = dlsym(handle, "hoge");
    hoge_func();
}
