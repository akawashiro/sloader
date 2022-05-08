#include "utils.h"

int LOG_LEVEL = INFO;

std::string LevelToString(int level) {
    switch (level) {
        case DEBUG:
            return "D";
        case INFO:
            return "I";
        case WARNING:
            return "W";
        case ERROR:
            return "E";
        case FATAL:
            return "F";
        default:
            std::abort();
    }
}

std::ofstream null_stream("/dev/null");

int StringToLevel(std::string s) {
    if (s == "DEBUG") return DEBUG;
    if (s == "INFO") return INFO;
    if (s == "WARNING") return WARNING;
    if (s == "ERROR") return ERROR;
    if (s == "FATAL") return FATAL;
    std::abort();
}

std::vector<std::string> SplitWith(std::string str,
                                   const std::string& delimiter) {
    std::vector<std::string> ret;
    size_t pos;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        ret.emplace_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    return ret;
}

std::string HexString(char* num, int length = -1) {
    if (length == -1) {
        length = 16;
    }
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(length)
       << std::hex << (u_int64_t)num;
    return ss.str();
}

std::string HexString(const char* num, int length = -1) {
    if (length == -1) {
        length = 16;
    }
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(length)
       << std::hex << (u_int64_t)num;
    return ss.str();
}

std::string ShowRelocationType(int type) {
    switch (type) {
        case R_X86_64_NONE:
            return "R_X86_64_NONE";
        case R_X86_64_64:
            return "R_X86_64_64";
        case R_X86_64_PC32:
            return "R_X86_64_PC32";
        case R_X86_64_GOT32:
            return "R_X86_64_GOT32";
        case R_X86_64_PLT32:
            return "R_X86_64_PLT32";
        case R_X86_64_COPY:
            return "R_X86_64_COPY";
        case R_X86_64_GLOB_DAT:
            return "R_X86_64_GLOB_DAT";
        case R_X86_64_JUMP_SLOT:
            return "R_X86_64_JUMP_SLOT";
        case R_X86_64_RELATIVE:
            return "R_X86_64_RELATIVE";
        case R_X86_64_GOTPCREL:
            return "R_X86_64_GOTPCREL";
        case R_X86_64_32:
            return "R_X86_64_32";
        case R_X86_64_32S:
            return "R_X86_64_32S";
        case R_X86_64_16:
            return "R_X86_64_16";
        case R_X86_64_PC16:
            return "R_X86_64_PC16";
        case R_X86_64_8:
            return "R_X86_64_8";
        case R_X86_64_PC8:
            return "R_X86_64_PC8";
        case R_X86_64_DTPMOD64:
            return "R_X86_64_DTPMOD64";
        case R_X86_64_DTPOFF64:
            return "R_X86_64_DTPOFF64";
        case R_X86_64_TPOFF64:
            return "R_X86_64_TPOFF64";
        case R_X86_64_TLSGD:
            return "R_X86_64_TLSGD";
        case R_X86_64_TLSLD:
            return "R_X86_64_TLSLD";
        case R_X86_64_DTPOFF32:
            return "R_X86_64_DTPOFF32";
        case R_X86_64_GOTTPOFF:
            return "R_X86_64_GOTTPOFF";
        case R_X86_64_TPOFF32:
            return "R_X86_64_TPOFF32";
        case R_X86_64_PC64:
            return "R_X86_64_PC64";
        case R_X86_64_GOTOFF64:
            return "R_X86_64_GOTOFF64";
        case R_X86_64_GOTPC32:
            return "R_X86_64_GOTPC32";
        case R_X86_64_GOT64:
            return "R_X86_64_GOT64";
        case R_X86_64_GOTPCREL64:
            return "R_X86_64_GOTPCREL64";
        case R_X86_64_GOTPC64:
            return "R_X86_64_GOTPC64";
        case R_X86_64_GOTPLT64:
            return "R_X86_64_GOTPLT64";
        case R_X86_64_PLTOFF64:
            return "R_X86_64_PLTOFF64";
        case R_X86_64_SIZE32:
            return "R_X86_64_SIZE32";
        case R_X86_64_SIZE64:
            return "R_X86_64_SIZE64";
        case R_X86_64_GOTPC32_TLSDESC:
            return "R_X86_64_GOTPC32_TLSDESC";
        case R_X86_64_TLSDESC:
            return "R_X86_64_TLSDESC";
        case R_X86_64_IRELATIVE:
            return "R_X86_64_IRELATIVE";
        case R_X86_64_RELATIVE64:
            return "R_X86_64_RELATIVE64";
        case R_X86_64_GOTPCRELX:
            return "R_X86_64_GOTPCRELX";
        case R_X86_64_REX_GOTPCRELX:
            return "R_X86_64_REX_GOTPCRELX";
        case R_X86_64_NUM:
            return "R_X86_64_NUM";
        default: {
            return HexString(type, 4);
        }
    }
}

std::string ShowRela(const Elf64_Rela& r) {
    std::stringstream ss;
    ss << "Elf_Rela{r_offset=" << HexString(r.r_offset, 8)
       << ", r_info=" << HexString(r.r_info, 8)
       << ", ELF64_R_SYM(r.r_info)=" << HexString(ELF64_R_SYM(r.r_info), 4)
       << ", ELF64_R_TYPE(r.r_info)="
       << ShowRelocationType(ELF64_R_TYPE(r.r_info))
       << ", r_addend=" << HexString(r.r_addend, 8) << "}";
    return ss.str();
}

std::string ShowSym(const Elf64_Sym& s, const char* strtab) {
    std::stringstream ss;
    std::string name = strtab + s.st_name;
    ss << "Elf64_Sym{st_name=" << name << "}";
    return ss.str();
}
