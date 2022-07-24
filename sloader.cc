#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "dyn_loader.h"
#include "exec_loader.h"
#include "utils.h"

Elf64_Half GetEType(const std::filesystem::path& filepath) {
    int fd = open(filepath.c_str(), O_RDONLY);
    CHECK(fd >= 0);

    size_t size = lseek(fd, 0, SEEK_END);
    CHECK_GT(size, 8UL + 16UL);

    size_t mapped_size = (size + 0xfff) & ~0xfff;

    char* p = (char*)mmap(NULL, mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE, fd, 0);
    CHECK(p != MAP_FAILED);

    Elf64_Ehdr* ehdr = reinterpret_cast<Elf64_Ehdr*>(p);
    return ehdr->e_type;
}

void ShowHelp(std::ostream& os) {
    os << "sloader -- Simple ELF loader" << std::endl;
}

int main(int argc, char* const argv[], char** envp) {
    google::InitGoogleLogging(argv[0]);

    static option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {0, 0, 0, 0},
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                ShowHelp(std::cout);
                return 0;
                break;
            default:
                LOG(INFO) << "Found unsupported option";
                CHECK(false);
                break;
        }
    }

    CHECK_LT(optind, argc);

    std::string argv0 = argv[optind++];
    std::filesystem::path fullpath;

    if (argv0[0] == '.' || argv0.find("/") != std::string::npos) {
        fullpath = std::filesystem::path(argv0);
    } else {
        std::vector<std::string> path_dirs =
            SplitWith(std::string(getenv("PATH")), ":");
        for (const auto& dir : path_dirs) {
            std::filesystem::path p = std::filesystem::path(dir) / argv0;
            if (std::filesystem::exists(p)) {
                fullpath = p;
                break;
            }
        }
    }

    LOG(INFO) << LOG_KEY(fullpath);
    CHECK(std::filesystem::exists(fullpath));

    std::vector<std::string> envs;
    for (char** env = envp; *env != 0; env++) {
        envs.emplace_back(*env);
    }

    Elf64_Half etype = GetEType(fullpath);
    if (etype == ET_EXEC) {
        auto exec_loader = MakeExecLoader(fullpath, argv0);
        exec_loader->Show();
        exec_loader->Load();
        exec_loader->Execute(envs);
        exec_loader->Unload();
    } else if (etype == ET_DYN) {
        auto dyn_loader = MakeDynLoader(fullpath, envs);
    } else {
        LOG(FATAL) << "Unsupported etype = " << LOG_KEY(etype);
    }
}
