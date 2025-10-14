#include "efi_string.h"

static CHAR16* u_str = (CHAR16[17]){0};
static const CHAR16* hexit = L"0123456789ABCDEF";

CHAR16* uint8_str(uint8_t uint) {
    int offset = 0;

    u_str[offset++] = hexit[0xF & (uint >> 4)];
    u_str[offset++] = hexit[0xF & (uint >> 0)];
    u_str[offset++] = 0;

    return u_str;
}

CHAR16* uint16_str(uint16_t uint) {
    int offset = 0;

    u_str[offset++] = hexit[0xF & (uint >> 12)];
    u_str[offset++] = hexit[0xF & (uint >> 8)];
    u_str[offset++] = hexit[0xF & (uint >> 4)];
    u_str[offset++] = hexit[0xF & (uint >> 0)];
    u_str[offset++] = 0;

    return u_str;
}

CHAR16* uint32_str(uint32_t uint) {
    int offset = 0;

    u_str[offset++] = hexit[0xF & (uint >> 28)];
    u_str[offset++] = hexit[0xF & (uint >> 24)];
    u_str[offset++] = hexit[0xF & (uint >> 20)];
    u_str[offset++] = hexit[0xF & (uint >> 16)];
    u_str[offset++] = hexit[0xF & (uint >> 12)];
    u_str[offset++] = hexit[0xF & (uint >> 8)];
    u_str[offset++] = hexit[0xF & (uint >> 4)];
    u_str[offset++] = hexit[0xF & (uint >> 0)];
    u_str[offset++] = 0;

    return u_str;
}

CHAR16* uint64_str(uint64_t uint) {
    int offset = 0;

    u_str[offset++] = hexit[0xF & (uint >> 60)];
    u_str[offset++] = hexit[0xF & (uint >> 56)];
    u_str[offset++] = hexit[0xF & (uint >> 52)];
    u_str[offset++] = hexit[0xF & (uint >> 48)];
    u_str[offset++] = hexit[0xF & (uint >> 44)];
    u_str[offset++] = hexit[0xF & (uint >> 40)];
    u_str[offset++] = hexit[0xF & (uint >> 36)];
    u_str[offset++] = hexit[0xF & (uint >> 32)];
    u_str[offset++] = hexit[0xF & (uint >> 28)];
    u_str[offset++] = hexit[0xF & (uint >> 24)];
    u_str[offset++] = hexit[0xF & (uint >> 20)];
    u_str[offset++] = hexit[0xF & (uint >> 16)];
    u_str[offset++] = hexit[0xF & (uint >> 12)];
    u_str[offset++] = hexit[0xF & (uint >> 8)];
    u_str[offset++] = hexit[0xF & (uint >> 4)];
    u_str[offset++] = hexit[0xF & (uint >> 0)];
    u_str[offset++] = 0;

    return u_str;
}
