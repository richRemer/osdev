#pragma once

#include <efi.h>
#include <stdint.h>
#include <stdarg.h>

CHAR16* fmt(CHAR16*, uint8_t, const CHAR16*, ...);
CHAR16* vfmt(CHAR16*, uint8_t, const CHAR16*, va_list);
