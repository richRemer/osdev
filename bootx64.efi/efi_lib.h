#pragma once

#include <stdarg.h>
#include <efi.h>

void efi_clear_screen();
void efi_cursor_disable();
void efi_cursor_enable();
void efi_cursor_to(uint8_t, uint8_t);
void efi_hang();
void efi_init(EFI_HANDLE, EFI_SYSTEM_TABLE*);
void efi_out(const CHAR16*, ...);
void efi_out_style(uint64_t, const CHAR16*, ...);
EFI_INPUT_KEY efi_read_key();
void efi_reset(EFI_RESET_TYPE);
void efi_reset_cold();
void efi_reset_shutdown();
void efi_reset_warm();
void efi_watchdog_disable();
