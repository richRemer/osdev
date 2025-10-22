const std = @import("std");
const io = @import("io.zig");
const config = @import("config.zig");
const elf = std.elf;
const unicode = std.unicode;
const uefi = std.os.uefi;
const Console = io.Console;
const SimpleFileSystem = uefi.protocol.SimpleFileSystem;
const File = uefi.protocol.File;

pub const BootError = error{
    ELFHeader,
    ELFLoadSegment,
    ELFNoCode,
    ELFProgramHeaders,
    KernelFound,
    LocateProtocol,
    MemoryMapInfo,
    MemoryMap,
    OpenKernel,
    OpenVolume,
    OutOfMemory,
};

pub fn main() void {
    load() catch |err| @panic(@errorName(err));
    @panic("loader returned unexpectedly");
}

fn load() BootError!void {
    var phys_address: u64 = 0x100000; // 1 MiB minimum base address
    var free_pages: u64 = 0;
    var vaddr_offset: u64 = 0;
    var kernel_address: u64 = 0;

    const allocator = uefi.pool_allocator;
    const in = uefi.system_table.con_in.?;
    const out = uefi.system_table.con_out.?;
    const console = Console.init(in, out);
    const boot = uefi.system_table.boot_services.?;
    const mmap_info = boot.getMemoryMapInfo() catch return BootError.MemoryMapInfo;
    const mmap_size = mmap_info.descriptor_size * mmap_info.len;
    const mmap_buffer = try allocator.alloc(u8, mmap_size);
    defer allocator.free(mmap_buffer);

    const mmap = boot.getMemoryMap(@alignCast(mmap_buffer)) catch |err| {
        console.printf("{s}\r\n", .{@errorName(err)});
        console.printf("{}\r\n", .{mmap_info});
        console.printf("{d}/{d}\r\n", .{ mmap_buffer.len, mmap_size });
        return BootError.MemoryMap;
    };

    // find highest physical address in map to place kernel
    var mmap_it = mmap.iterator();
    while (mmap_it.next()) |mem| {
        if (config.debug) {
            console.printf("mem {}\r\n", .{mem});
        }

        if (mem.type == .conventional_memory and mem.physical_start >= phys_address) {
            phys_address = mem.physical_start;
            free_pages = mem.number_of_pages;
        }
    }

    const protocol = boot.locateProtocol(SimpleFileSystem, null) catch return BootError.LocateProtocol;
    const disk = protocol.?;
    const esp = disk.openVolume() catch return BootError.OpenVolume;
    defer esp.close() catch {};

    const kernel_path = unicode.utf8ToUtf16LeStringLiteral("\\kernel.bin");
    const kernel_file = esp.open(kernel_path, .read, .{ .read_only = true }) catch return BootError.OpenKernel;
    defer kernel_file.close() catch {};

    const elf_header = read_elf_header(kernel_file) catch return BootError.ELFHeader;
    const prog_headers_size = elf_header.phnum * elf_header.phentsize;
    const headers_size = elf_header.phoff + prog_headers_size;
    const headers_buffer = try allocator.alloc(u8, headers_size);
    defer allocator.free(headers_buffer);

    const prog_headers_buffer = try allocator.alloc(u8, prog_headers_size);
    defer allocator.free(prog_headers_buffer);

    var ph_it = elf_header.iterateProgramHeadersBuffer(headers_buffer);
    var first_segment = true;
    while (ph_it.next() catch return BootError.ELFProgramHeaders) |segment| {
        if (segment.p_type == elf.PT_LOAD) {
            // assume first loaded segment is kernel
            if (first_segment) {
                kernel_address = segment.p_vaddr;
                vaddr_offset = kernel_address - phys_address;
                first_segment = false;
            }

            load_segment(
                kernel_file,
                segment.p_offset,
                segment.p_filesz,
                segment.p_memsz,
                segment.p_vaddr - vaddr_offset,
            ) catch return BootError.ELFLoadSegment;
        }
    }

    if (first_segment) {
        return BootError.ELFNoCode;
    }

    return BootError.KernelFound;
}

fn load_segment(
    /// open ELF file
    file: *File,
    /// offset into file where segment begins
    offset: u64,
    /// length of segment in file
    length: usize,
    /// size of segment in memory
    size: usize,
    /// virtual address of segment
    vaddr: u64,
) !void {
    if (vaddr & 0xFFF != 0) return error.SegmentUnaligned;
    if (length > size) return error.SegmentUnfit;

    const boot = uefi.system_table.boot_services.?;
    const pages = try boot.allocatePages(
        .{ .address = @ptrFromInt(vaddr) },
        .loader_data,
        size >> 12,
    );
    errdefer boot.freePages(pages) catch {};

    var segment: []u8 = &.{};

    segment.ptr = @ptrCast(pages.ptr);
    segment.len = pages.len * 4096;

    try file.setPosition(offset);
    _ = try file.read(segment[0..length]);

    const zero_start: u64 = vaddr + length;
    const zero_count: u64 = size - length;

    if (zero_count > 0) {
        @memset(@as([*]u8, @ptrFromInt(zero_start))[0..zero_count], 0);
    }
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
