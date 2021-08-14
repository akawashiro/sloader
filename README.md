# sloader
Simple loader
```
mkdir build
cd build
cmake .. -G Ninja
ninja
ctest -V
```

クラッシュ時のbacktrace
```
#0  0x0000000000420df0 in ?? () // __memcmp_sse2
#1  0x0000000000443dfc in ?? () // _dl_tunable_set_x86_shstk
#2  0x0000000000001000 in ?? ()
#3  0x0000000000402cb4 in ?? () // __libc_start_main (call __tunable_get_val)
#4  0x0000000100000000 in ?? ()
#5  0x00007fffffffe048 in ?? ()
#6  0x00000000004017e5 in ?? () // main
#7  0x000000000040103e in ?? () // .plt
#8  0x0000000000008000 in ?? ()
#9  0x0000000000008000 in ?? ()
#10 0x0000000000000040 in ?? ()
#11 0x0000000000000004 in ?? ()
#12 0x0000000000000040 in ?? ()
#13 0x0000000000000010 in ?? ()
#14 0x0000000000486865 in ?? ()
#15 0x0000000000000100 in ?? ()
#16 0x0000000000486865 in ?? ()
```
- `0x00000000004017e5` はbacktraceに現れてはいるものの、breakpointを設定しても止まらない。このことから、return時に呼び出されると考えられる。
- `ARCH_INIT_CPU_FEATURES` の中で死んでる?
[https://github.com/bminor/glibc/blob/1f51cd9a860ee45eee8a56fb2ba925267a2a7bfe/sysdeps/unix/sysv/linux/aarch64/libc-start.c#L30]()
- [https://github.com/bminor/glibc/blob/1f51cd9a860ee45eee8a56fb2ba925267a2a7bfe/sysdeps/x86/cpu-features.c#L792-L798]()
    - `_dl_tunable_set_x86_shstk` と `_dl_tunable_set_x86_ibt` が並んでいるのでこのあたりが原因
- [https://github.com/bminor/glibc/blob/1f51cd9a860ee45eee8a56fb2ba925267a2a7bfe/elf/dl-tunables.h#L84-L85]()
    - Get and return a tunable value.  If the tunable was set externally and `__CB` is defined then call `__CB` before returning the value.
    - コメントに対応するところ: [https://github.com/bminor/glibc/blob/1f51cd9a860ee45eee8a56fb2ba925267a2a7bfe/elf/dl-tunables.c#L432-L433]()
- 死ぬ直前 [https://github.com/bminor/glibc/blob/1f51cd9a860ee45eee8a56fb2ba925267a2a7bfe/sysdeps/x86/cpu-tunables.c#L312-L323]()
