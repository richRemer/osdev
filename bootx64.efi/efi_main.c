#include <efi.h>
#include <efilib.h>
#include "efi_string.h"

static CHAR16* Booting = L"Booting custom EFI\r\n";
static CHAR16* DisplayMode = L"Mode %d ( %q x %q )\r\n";
static CHAR16* MapLoaded = L"Memory map loaded\r\n";
static CHAR16* MemDescriptor = L"Memory Descriptor:\r\n";

static void enable_cursor(EFI_SYSTEM_TABLE*);
static void print_mem_map(EFI_SYSTEM_TABLE*);
static void print_mode_info(EFI_SYSTEM_TABLE*);
static void print_modes(EFI_SYSTEM_TABLE*);
static void wait_forever();

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    SIMPLE_TEXT_OUTPUT_INTERFACE* out = SystemTable->ConOut;

    out->ClearScreen(out);
    out->OutputString(out, Booting);

    print_mode_info(SystemTable);
    print_modes(SystemTable);
    print_mem_map(SystemTable);
    enable_cursor(SystemTable);
    wait_forever();

    return EFI_SUCCESS;
}

static void enable_cursor(EFI_SYSTEM_TABLE* SystemTable) {
    SIMPLE_TEXT_OUTPUT_INTERFACE* out = SystemTable->ConOut;

    out->EnableCursor(out, TRUE);
}

static void print_mem_map(EFI_SYSTEM_TABLE* SystemTable) {
    SIMPLE_TEXT_OUTPUT_INTERFACE* out = SystemTable->ConOut;
    UINTN MemoryMapSize = 0;
    UINTN MapKey = 0;
    UINTN DescriptorSize = 0;
    UINT32 DescriptorVersion = 0;
    EFI_MEMORY_DESCRIPTOR* MemoryMap;
    EFI_STATUS Status;
    CHAR16 buf[100];

    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
        NULL, &MapKey, &DescriptorSize, &DescriptorVersion);

    if (Status != EFI_BUFFER_TOO_SMALL) {
        // can't get buffer size; just bail out for now
        return;
    }

    // considered good practice in case map grows between calls
    MemoryMapSize += 2*DescriptorSize;

    Status = SystemTable->BootServices->AllocatePool(EfiLoaderData,
        MemoryMapSize, (void**)&MemoryMap);

    if (EFI_ERROR(Status)) {
        // failed to allocate memory
        return;
    }

    Status = SystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
        MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

    if (EFI_ERROR(Status)) {
        // failed to load map
        SystemTable->BootServices->FreePool(MemoryMap);
        return;
    }

    out->OutputString(out, MapLoaded);
    out->OutputString(out, fmt(buf, 100, L"Map Key: %q\r\n", MapKey));
    out->OutputString(out, fmt(buf, 100, L"Descriptor Size: %q\r\n", DescriptorSize));

    EFI_MEMORY_DESCRIPTOR* MemDesc = MemoryMap;

    for (int i=0; i<MemoryMapSize/DescriptorSize; i++) {
        out->OutputString(out, MemDescriptor);
        out->OutputString(out, fmt(buf, 100, L"-Type: %d\r\n", MemDesc->Type));
        out->OutputString(out, fmt(buf, 100, L"-Physical Start: %Q\r\n", MemDesc->PhysicalStart));
        out->OutputString(out, fmt(buf, 100, L"-Virtual Start: %Q\r\n", MemDesc->VirtualStart));
        out->OutputString(out, fmt(buf, 100, L"-Number Of Pages: %q\r\n", MemDesc->NumberOfPages));
        out->OutputString(out, fmt(buf, 100, L"-Attribute: %q\r\n", MemDesc->Attribute));
    }
}

static void print_mode_info(EFI_SYSTEM_TABLE* SystemTable) {
    SIMPLE_TEXT_OUTPUT_INTERFACE* out = SystemTable->ConOut;
    SIMPLE_TEXT_OUTPUT_MODE* mode = out->Mode;
    CHAR16 buf[100];

    out->OutputString(out, fmt(buf, 100, L"Attribute: %d\r\n", mode->Attribute));
    out->OutputString(out, fmt(buf, 100, L"CursorColumn: %d\r\n", mode->CursorColumn));
    out->OutputString(out, fmt(buf, 100, L"CursorRow: %d\r\n", mode->CursorRow));
    out->OutputString(out, fmt(buf, 100, L"CursorVisible: %b\r\n", mode->CursorVisible));
    out->OutputString(out, fmt(buf, 100, L"MaxMode: %d\r\n", mode->MaxMode));
    out->OutputString(out, fmt(buf, 100, L"Mode: %d\r\n", mode->Mode));
}

static void print_modes(EFI_SYSTEM_TABLE* SystemTable) {
    SIMPLE_TEXT_OUTPUT_INTERFACE* out = SystemTable->ConOut;
    CHAR16 buf[100];

    for (INT32 mode=0; mode<=out->Mode->MaxMode; mode++) {
        UINTN cols;
        UINTN rows;

        out->QueryMode(out, mode, &cols, &rows);

        fmt(buf, 100, L"-Mode %d ( %q x %q )\r\n", mode, cols, rows);

        out->OutputString(out, buf);
    }
}

static void wait_forever() {
    for (;;) ;
}

// TODO: figure out how StdErr works
// TODO: the following does nothing
//  SIMPLE_TEXT_OUTPUT_INTERFACE* err = SystemTable->StdErr;
//  err->OutputString(err, L"bootloader does nothing\r\n");
//
// TODO: figure out how TestString works
// TODO: the following does nothing
//  out->TestString(out, L"Test String\r\n");
