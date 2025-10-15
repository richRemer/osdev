#include "efi_string.h"

static const CHAR16* hexit = L"0123456789ABCDEF";
static const CHAR16* digit = L"0123456789";

static uint8_t btou(CHAR16*, uint8_t, uint8_t, uint8_t);
static uint8_t dtou(CHAR16*, uint8_t, uint8_t, uint32_t);
static uint8_t itou(CHAR16*, uint8_t, uint8_t, int64_t);
static uint8_t qtou(CHAR16*, uint8_t, uint8_t, uint64_t);
static uint8_t utou(CHAR16*, uint8_t, uint8_t, uint64_t);
static uint8_t wtou(CHAR16*, uint8_t, uint8_t, uint16_t);
static uint8_t xtou(CHAR16*, uint8_t, uint8_t, uint64_t, uint8_t);

CHAR16* fmt(CHAR16* buffer, uint8_t size, const CHAR16* format, ...) {
    int f_idx = 0;
    int d_idx = 0;

    va_list args;
    va_start(args, format);

    while (d_idx < size) {
        if (format[f_idx] == '%') {
            switch (format[++f_idx]) {
                case '%': buffer[d_idx++] = '%'; break;
                case 'b': d_idx = utou(buffer, size, d_idx, va_arg(args, int)); break;
                case 'B': d_idx = btou(buffer, size, d_idx, va_arg(args, int)); break;
                case 'w': d_idx = utou(buffer, size, d_idx, va_arg(args, int)); break;
                case 'W': d_idx = wtou(buffer, size, d_idx, va_arg(args, int)); break;
                case 'd': d_idx = utou(buffer, size, d_idx, va_arg(args, uint32_t)); break;
                case 'D': d_idx = dtou(buffer, size, d_idx, va_arg(args, uint32_t)); break;
                case 'q': d_idx = utou(buffer, size, d_idx, va_arg(args, uint64_t)); break;
                case 'Q': d_idx = qtou(buffer, size, d_idx, va_arg(args, uint64_t)); break;
                default:
                    buffer[d_idx++] = '%';
                    buffer[d_idx++] = format[f_idx];
            }

            f_idx++;
        } else if (format[f_idx] == '\0') {
            buffer[d_idx++] = format[f_idx++];
            break;
        } else {
            buffer[d_idx++] = format[f_idx++];
        }
    }

    va_end(args);

    // ensure buffer always null-terminated
    buffer[size-1] = '\0';

    return buffer;
}

static uint8_t btou(CHAR16* buffer, uint8_t size, uint8_t offset, uint8_t byte) {
    return xtou(buffer, size, offset, byte, 1);
}

static uint8_t dtou(CHAR16* buffer, uint8_t size, uint8_t offset, uint32_t dword) {
    return xtou(buffer, size, offset, dword, 4);
}

static uint8_t itou(CHAR16* buffer, uint8_t size, uint8_t offset, int64_t sint) {
    if (offset < size && sint < 0) {
        buffer[offset++] = '-';
    }

    return utou(buffer, size, offset, sint < 0 ? -sint : sint);
}

static uint8_t qtou(CHAR16* buffer, uint8_t size, uint8_t offset, uint64_t qword) {
    return xtou(buffer, size, offset, qword, 8);
}

static uint8_t utou(CHAR16* buffer, uint8_t size, uint8_t offset, uint64_t uint) {
    CHAR16 chars[20];
    int charidx = 20;

    if (uint == 0 && offset < size) {
        buffer[offset++] = '0';
    } else {
        while (uint > 0) {
            uint8_t value = uint % 10;
            uint /= 10;
            chars[--charidx] = digit[value];
        }

        while (charidx < 20 && offset < size) {
            buffer[offset++] = chars[charidx++];
        }
    }

    return offset;
}

static uint8_t wtou(CHAR16* buffer, uint8_t size, uint8_t offset, uint16_t word) {
    return xtou(buffer, size, offset, word, 2);
}

static uint8_t xtou(CHAR16* buffer, uint8_t size, uint8_t offset, uint64_t value, uint8_t octets) {
    uint8_t byte;

    while (offset < size && octets > 0) {
        byte = value >> (--octets * 8);
        buffer[offset++] = hexit[byte >> 4];
        if (offset < size) buffer[offset++] = hexit[byte & 0xF];
    }

    return offset;
}
