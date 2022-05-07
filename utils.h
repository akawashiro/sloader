#pragma once

#include <elf.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

constexpr int DEBUG = 0;
constexpr int INFO = 1;
constexpr int WARNING = 2;
constexpr int ERROR = 3;
constexpr int FATAL = 4;

std::string LevelToString(int level);

int StringToLevel(std::string s);

extern int LOG_LEVEL;

extern std::ofstream null_stream;

// TODO(akawashiro): Emit std::endl at the end of each message. Not the head!.
// TODO(akawashiro): After implementing FATAL, remove all std::abort().
#define LOG(level)                                   \
    ((LOG_LEVEL <= level) ? std::cerr : null_stream) \
        << std::endl                                 \
        << LevelToString(level) << " " << __FILE__ << ":" << __LINE__ << " "
#define LOG_KEY_VALUE(key, value) key << "=" << value << " "
#define LOG_KEY(key) LOG_KEY_VALUE(#key, key)
#define LOG_BITS(key) LOG_KEY_VALUE(#key, HexString(key))
#define LOG_16BITS(key) LOG_KEY_VALUE(#key, HexString(key, 4))
#define LOG_32BITS(key) LOG_KEY_VALUE(#key, HexString(key, 8))

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

std::string HexString(char* num, int length);

std::string HexString(const char* num, int length);

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
                                   const std::string& delimiter);
