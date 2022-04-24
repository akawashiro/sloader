#pragma once

constexpr int DEBUG = 0;
constexpr int INFO = 1;
constexpr int WARNING = 2;
constexpr int ERROR = 3;
constexpr int FATAL = 4;

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

int StringToLevel(std::string s) {
    if (s == "DEBUG") return DEBUG;
    if (s == "INFO") return INFO;
    if (s == "WARNING") return WARNING;
    if (s == "ERROR") return ERROR;
    if (s == "FATAL") return FATAL;
    std::abort();
}

int LOG_LEVEL = INFO;

std::ofstream null_stream("/dev/null");

// TODO(akawashiro): Emit std::endl at the end of each message. Not the head!.
// TODO(akawashiro): After implementing FATAL, remove all std::abort().
#define LOG(level)                                   \
    ((LOG_LEVEL <= level) ? std::cerr : null_stream) \
        << std::endl                                 \
        << LevelToString(level) << " " << __FILE__ << ":" << __LINE__ << " "
#define LOG_KEY_VALUE(key, value) key << "=" << value << " "
#define LOG_KEY(key) LOG_KEY_VALUE(#key, key)
#define LOG_BITS(key) LOG_KEY_VALUE(#key, HexString(key))

#define CHECK(cond)                       \
    if (!(cond)) {                        \
        LOG(ERROR) << #cond << std::endl; \
        std::abort();                     \
    }
#define CHECK_EQ(a, b)                                              \
    if ((a) != (b)) {                                               \
        LOG(ERROR) << #a << "(" << a << ")"                         \
                   << " != " << #b << "(" << b << ")" << std::endl; \
        std::abort();                                               \
    }
#define CHECK_NE(a, b)                                              \
    if ((a) == (b)) {                                               \
        LOG(ERROR) << #a << "(" << a << ")"                         \
                   << " == " << #b << "(" << b << ")" << std::endl; \
        std::abort();                                               \
    }
#define CHECK_GT(a, b)                                              \
    if ((a) <= (b)) {                                               \
        LOG(ERROR) << #a << "(" << a << ")"                         \
                   << " <= " << #b << "(" << b << ")" << std::endl; \
        std::abort();                                               \
    }
#define CHECK_LE(a, b)                                             \
    if ((a) > (b)) {                                               \
        LOG(ERROR) << #a << "(" << a << ")"                        \
                   << " > " << #b << "(" << b << ")" << std::endl; \
        std::abort();                                              \
    }
#define CHECK_LT(a, b)                                              \
    if ((a) >= (b)) {                                               \
        LOG(ERROR) << #a << "(" << a << ")"                         \
                   << " >= " << #b << "(" << b << ")" << std::endl; \
        std::abort();                                               \
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

template <class T>
std::string HexString(T num, int length = -1) {
    if (length == -1) {
        length = sizeof(T) * 2;
    }
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(length)
       << std::hex << +num;
    return ss.str();
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

