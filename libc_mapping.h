#pragma once

#include <elf.h>
#include <map>
#include <string>

namespace libc_mapping {
extern std::map<std::string, Elf64_Addr> sloader_libc_map;
}
