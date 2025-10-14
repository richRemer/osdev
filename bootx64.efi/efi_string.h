#pragma once

#include <efi.h>
#include <stdint.h>
#include <stdarg.h>

CHAR16* fmt(CHAR16*, uint8_t, const CHAR16*, ...);
