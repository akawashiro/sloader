#include <stdio.h>
#include <sys/auxv.h>

int main(int argc, char** argv, char** envp) {
    char* aux_names[] = {
        "AT_NULL",           "AT_IGNORE",
        "AT_EXECFD",         "AT_PHDR",
        "AT_PHENT",          "AT_PHNUM",
        "AT_PAGESZ",         "AT_BASE",
        "AT_FLAGS",          "AT_ENTRY",
        "AT_NOTELF",         "AT_UID",
        "AT_EUID",           "AT_GID",
        "AT_EGID",           "AT_CLKTCK",
        "AT_PLATFORM",       "AT_HWCAP",
        "AT_FPUCW",          "AT_DCACHEBSIZE",
        "AT_ICACHEBSIZE",    "AT_UCACHEBSIZE",
        "AT_IGNOREPPC",      "AT_SECURE",
        "AT_BASE_PLATFORM",  "AT_RANDOM",
        "AT_HWCAP2",         "AT_EXECFN",
        "AT_SYSINFO",        "AT_SYSINFO_EHDR",
        "AT_L1I_CACHESHAPE", "AT_L1D_CACHESHAPE",
        "AT_L2_CACHESHAPE",  "AT_L3_CACHESHAPE",
        "AT_L1I_CACHESIZE",  "AT_L1I_CACHEGEOMETRY",
        "AT_L1D_CACHESIZE",  "AT_L1D_CACHEGEOMETRY",
        "AT_L2_CACHESIZE",   "AT_L2_CACHEGEOMETRY",
        "AT_L3_CACHESIZE",   "AT_L3_CACHEGEOMETRY",
        "AT_MINSIGSTKSZ",
    };
    unsigned long aux_types[] = {AT_NULL,           AT_IGNORE,
                                 AT_EXECFD,         AT_PHDR,
                                 AT_PHENT,          AT_PHNUM,
                                 AT_PAGESZ,         AT_BASE,
                                 AT_FLAGS,          AT_ENTRY,
                                 AT_NOTELF,         AT_UID,
                                 AT_EUID,           AT_GID,
                                 AT_EGID,           AT_CLKTCK,
                                 AT_PLATFORM,       AT_HWCAP,
                                 AT_FPUCW,          AT_DCACHEBSIZE,
                                 AT_ICACHEBSIZE,    AT_UCACHEBSIZE,
                                 AT_IGNOREPPC,      AT_SECURE,
                                 AT_BASE_PLATFORM,  AT_RANDOM,
                                 AT_HWCAP2,         AT_EXECFN,
                                 AT_SYSINFO,        AT_SYSINFO_EHDR,
                                 AT_L1I_CACHESHAPE, AT_L1D_CACHESHAPE,
                                 AT_L2_CACHESHAPE,  AT_L3_CACHESHAPE,
                                 AT_L1I_CACHESIZE,  AT_L1I_CACHEGEOMETRY,
                                 AT_L1D_CACHESIZE,  AT_L1D_CACHEGEOMETRY,
                                 AT_L2_CACHESIZE,   AT_L2_CACHEGEOMETRY,
                                 AT_L3_CACHESIZE,   AT_L3_CACHEGEOMETRY,
                                 AT_MINSIGSTKSZ};
    int n_aux_types = 43;
    for (int i = 0; i < n_aux_types; i++) {
        printf("%s = %lu\n", aux_names[i], getauxval(aux_types[i]));
    }

    for (char** env = envp; *env != 0; env++) {
        char* thisEnv = *env;
        printf("%s\n", thisEnv);
    }
    // TODO: Need this?
    fflush(stdout);
    return 0;
}
