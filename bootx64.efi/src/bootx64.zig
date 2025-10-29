const std = @import("std");
const io = @import("io.zig");
const config = @import("config.zig");
const elf = std.elf;
const unicode = std.unicode;
const uefi = std.os.uefi;
const Allocator = std.mem.Allocator;
const Console = io.Console;
const SimpleFileSystem = uefi.protocol.SimpleFileSystem;
const File = uefi.protocol.File;

const page_size = 4096;

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

fn flatten(pages: [][page_size]u8) []u8 {
    var flat: []u8 = &.{};

    flat.ptr = @ptrCast(pages.ptr);
    flat.len = pages.len * page_size;

    return flat;
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

    console.log("bootloader started", .{});

    const boot = uefi.system_table.boot_services.?;
    const mmap_info = boot.getMemoryMapInfo() catch |err| {
        console.err("failed to retrieve memory map info", .{});
        console.err("{s}", .{@errorName(err)});
        return BootError.MemoryMapInfo;
    };
    console.dbg("retrieved memory map info", .{});

    const mmap_size = mmap_info.descriptor_size * mmap_info.len;
    const mmap_pages = boot.allocatePages(
        .any,
        .loader_data,
        (mmap_size + page_size - 1) / page_size,
    ) catch |err| {
        console.err("could not allocate pages for memory map", .{});
        console.err("{s}", .{@errorName(err)});
        return BootError.OutOfMemory;
    };
    defer boot.freePages(mmap_pages) catch {};
    console.dbg("memory map buffer allocated", .{});

    const mmap_buffer = flatten(mmap_pages);
    const mmap = boot.getMemoryMap(@alignCast(mmap_buffer)) catch |err| {
        console.err("failed to get memory map", .{});
        console.err("{s}", .{@errorName(err)});
        return BootError.MemoryMap;
    };
    console.dbg("memory map loaded", .{});

    var mmap_it = mmap.iterator();
    while (mmap_it.next()) |mem| {
        switch (mem.type) {
            // ACPI reserved
            .reserved_memory_type,
            .runtime_services_code,
            .runtime_services_data,
            .unusable_memory,
            .memory_mapped_io,
            .memory_mapped_io_port_space,
            .pal_code,
            // usable after reading ACPI tables
            .acpi_reclaim_memory,
            // need to map, but not usable
            .acpi_memory_nvs,
            // non-volatile storage for EFI
            .persistent_memory,
            => {},
            // usable
            .loader_code,
            .loader_data,
            .boot_services_code,
            .boot_services_data,
            .conventional_memory,
            => {
                console.dbg(
                    "{X} @ {X} [attr: {X}] ({} pages) {s}",
                    .{
                        mem.physical_start,
                        mem.virtual_start,
                        @as(u64, @bitCast(mem.attribute)),
                        mem.number_of_pages,
                        @tagName(mem.type),
                    },
                );
            },
            // remaining types reserved for OS, OEM, or future use
            else => {},
        }

        if (mem.type == .conventional_memory and mem.physical_start >= phys_address) {
            phys_address = mem.physical_start;
            free_pages = mem.number_of_pages;
        }
    }
    console.dbg("determined physical address of kernel", .{});

    const protocol = boot.locateProtocol(SimpleFileSystem, null) catch |err| {
        console.err("failed to locate Simple File System protocol", .{});
        console.err("{s}", .{@errorName(err)});
        return BootError.LocateProtocol;
    };
    const disk = protocol.?;
    console.dbg("located Simple File System protocol", .{});

    const esp = disk.openVolume() catch |err| {
        console.err("failed to open EFI System Partition", .{});
        console.err("{s}", .{@errorName(err)});
        return BootError.OpenVolume;
    };
    defer esp.close() catch {};
    console.dbg("opened EFI System Partition", .{});

    const kernel_path = unicode.utf8ToUtf16LeStringLiteral("\\kernel.bin");
    const kernel_file = esp.open(kernel_path, .read, .{ .read_only = true }) catch |err| {
        console.err("failed to open kernel", .{});
        console.err("{s}", .{@errorName(err)});
        return BootError.OpenKernel;
    };
    defer kernel_file.close() catch {};
    console.dbg("opened kernel", .{});

    const elf_header = read_elf_header(kernel_file) catch |err| {
        console.err("could not read ELF header from kernel", .{});
        console.err("{s}", .{@errorName(err)});
        return BootError.ELFHeader;
    };
    console.dbg("kernel ELF header read", .{});

    const prog_headers_size = elf_header.phnum * elf_header.phentsize;
    const headers_size = elf_header.phoff + prog_headers_size;
    const headers_buffer = try allocator.alloc(u8, headers_size);
    defer allocator.free(headers_buffer);
    console.dbg("allocated buffer for program headers", .{});

    var ph_it = elf_header.iterateProgramHeadersBuffer(headers_buffer);
    var first_segment = true;
    console.dbg("iterating over program headers", .{});
    while (ph_it.next() catch return BootError.ELFProgramHeaders) |segment| {
        if (segment.p_type == elf.PT_LOAD) {
            console.dbg("LOAD: {}", .{segment.p_vaddr});

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
            ) catch |err| {
                console.err("could not load segment", .{});
                console.err("{s}", .{@errorName(err)});
                return BootError.ELFLoadSegment;
            };
        } else {
            console.dbg("????: {}", .{segment.p_vaddr});
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
    segment.len = pages.len * page_size;

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
