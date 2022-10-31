// This file is to secure memory space just after %fs register.

constexpr int TLS_SPACE_FOR_LOADEE = 4096;
thread_local unsigned char sloader_dummy_to_secure_tls_space[TLS_SPACE_FOR_LOADEE] = {0, 0, 0, 0};
unsigned long sloader_tls_offset = 4096;

void write_sloader_dummy_to_secure_tls_space() {
    sloader_dummy_to_secure_tls_space[0] = 0xaa;
    sloader_dummy_to_secure_tls_space[1] = 0xaa;
    sloader_dummy_to_secure_tls_space[2] = 0xaa;
    sloader_dummy_to_secure_tls_space[3] = 0xaa;
    sloader_dummy_to_secure_tls_space[TLS_SPACE_FOR_LOADEE - 4] = 0xab;
    sloader_dummy_to_secure_tls_space[TLS_SPACE_FOR_LOADEE - 3] = 0xcd;
    sloader_dummy_to_secure_tls_space[TLS_SPACE_FOR_LOADEE - 2] = 0xab;
    sloader_dummy_to_secure_tls_space[TLS_SPACE_FOR_LOADEE - 1] = 0xcd;
}
