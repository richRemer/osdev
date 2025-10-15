#pragma once

#include <stdarg.h>
#include <efi.h>

void efi_clear_screen();
void efi_cursor_disable();
void efi_cursor_enable();
void efi_hang();
void efi_init(EFI_HANDLE, EFI_SYSTEM_TABLE*);
void efi_out(const CHAR16*, ...);
EFI_INPUT_KEY efi_read_key();
