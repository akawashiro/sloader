// This file is to secure memory space just after %fs register.

namespace {
constexpr int TLS_SPACE_FOR_LOADEE = 4;
thread_local volatile unsigned long sloader_dummy_to_secure_tls_space[TLS_SPACE_FOR_LOADEE] = {0, 0, 0, 0};
}  // namespace

void write_sloader_dummy_to_secure_tls_space() {
    for (int i = 0; i < TLS_SPACE_FOR_LOADEE; i++) {
        sloader_dummy_to_secure_tls_space[i] = 0xdeadbeefdeadbeef;
    }
}
