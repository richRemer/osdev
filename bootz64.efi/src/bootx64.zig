const std = @import("std");
const elf = std.elf;
const unicode = std.unicode;
const uefi = std.os.uefi;
const SimpleFileSystem = uefi.protocol.SimpleFileSystem;
const File = uefi.protocol.File;

pub fn main() void {
    load() catch |err| @panic(@errorName(err));
    @panic("loader returned unexpectedly");
}

fn load() !void {
    var base_address: u64 = 0x100000; // 1 MiB minimum base address
    var free_pages: u64 = 0;

    const allocator = uefi.pool_allocator;
    const boot = uefi.system_table.boot_services.?;
    const mmap_info = try boot.getMemoryMapInfo();
    const mmap_size = mmap_info.descriptor_size * mmap_info.len;
    const mmap_buffer = try allocator.alloc(u8, mmap_size);
    defer allocator.free(mmap_buffer);

    const mmap = try boot.getMemoryMap(@alignCast(mmap_buffer));

    // find highest physical address in map to place kernel
    var mmap_it = mmap.iterator();
    while (mmap_it.next()) |mem| {
        if (mem.type == .conventional_memory and mem.physical_start >= base_address) {
            base_address = mem.physical_start;
            free_pages = mem.number_of_pages;
        }
    }

    const disk = (try boot.locateProtocol(SimpleFileSystem, null)).?;
    const esp = try disk.openVolume();
    defer esp.close() catch {};

    const kernel_path = unicode.utf8ToUtf16LeStringLiteral("\\kernel.bin");
    const kernel_file = try esp.open(kernel_path, .read, .{ .read_only = true });
    defer kernel_file.close() catch {};

    const elf_header = try read_elf_header(kernel_file);
    const prog_headers_size = elf_header.phnum * elf_header.phentsize;
    const headers_size = elf_header.phoff + prog_headers_size;
    const headers_buffer = try allocator.alloc(u8, headers_size);
    defer allocator.free(headers_buffer);

    const prog_headers_buffer = try allocator.alloc(u8, prog_headers_size);
    defer allocator.free(prog_headers_buffer);

    var ph_it = elf_header.iterateProgramHeadersBuffer(headers_buffer);
    while (try ph_it.next()) |section| {
        _ = section;
    }

    return error.KernelFound;
}

fn read_elf_header(file: *File) !elf.Header {
    var header: elf.Elf64_Ehdr = undefined;
    const ptr: [*]u8 = @ptrCast(&header);
    const slice: []u8 = ptr[0..@sizeOf(elf.Elf64_Ehdr)];

    try file.setPosition(0);
    _ = try file.read(slice);

    var reader: std.Io.Reader = .fixed(slice);
    return try elf.Header.read(&reader);
}
