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
