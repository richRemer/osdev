const std = @import("std");
const elf = std.elf;
const uefi = std.os.uefi;
const unicode = std.unicode;
const Allocator = std.mem.Allocator;
const File = uefi.protocol.File;
const SimpleFileSystem = uefi.protocol.SimpleFileSystem;
const BootServices = uefi.tables.BootServices;
const MemoryMapInfo = uefi.tables.MemoryMapInfo;
const MemoryMapSlice = uefi.tables.MemoryMapSlice;
const Console = @import("io.zig").Console;

const EFI_PAGE_SIZE = 4096;
const KERNEL_PATH = "\\kernel.bin";

/// Boot loader state and implementation.
pub const Loader = struct {
    kernel_paddr: u64,
    kernel_vaddr: u64,
    boot: *BootServices,
    output: Logger,
    allocator: Allocator,
    elf_header: elf.Header,
    mmap: MemoryMapSlice,
    mmap_info: MemoryMapInfo,
    mmap_pages: ?[][EFI_PAGE_SIZE]u8 = null,
    esp: ?*File = null,
    kernel: ?*File = null,
    kernel_buf: ?[]u8 = null,

    pub fn init() Loader {
        const boot = uefi.system_table.boot_services.?;
        const in = uefi.system_table.con_in.?;
        const out = uefi.system_table.con_out.?;
        const console = Console.init(in, out);

        return .{
            .kernel_paddr = 0x100000, // 1 MiB minimum
            .kernel_vaddr = 0,
            .boot = boot,
            .output = Logger.init(console),
            .allocator = uefi.pool_allocator,
            .elf_header = undefined,
            .mmap = undefined,
            .mmap_info = undefined,
        };
    }

    pub fn deinit(this: *Loader) void {
        if (this.mmap_pages) |pages| {
            if (this.esp) |root| {
                if (this.kernel) |file| {
                    if (this.kernel_buf) |buf| {
                        this.allocator.free(buf);
                        this.kernel_buf = null;
                    }

                    file.close();
                    this.kernel = null;
                }

                root.close();
                this.esp = null;
            }

            this.boot.freePages(pages);
            this.mmap_pages = null;
        }
    }

    pub fn load(this: *Loader) !void {
        this.output.beginSection("bootloader");

        try this.mmapInfo();
        try this.mmapPages();
        try this.mmapLoad();
        try this.kernelAddress();
        try this.fileSystem();
        try this.openKernel();
        try this.readELFHeader();
        try this.prepareForProgramHeaders();
        try this.readProgramHeaders();

        this.output.flushSection();
        this.output.beginSection("kernel segments");

        try this.loadSegments();

        this.output.flushSection();
    }

    fn fileSystem(this: *Loader) !void {
        this.output.beginTask("open EFI System Partition");
        errdefer this.output.failed();

        const protocol = try this.boot.locateProtocol(SimpleFileSystem, null);

        if (protocol) |fs| {
            this.esp = try fs.openVolume();
        } else {
            return error.LocateError;
        }

        this.output.ok();
    }

    fn kernelAddress(this: *Loader) !void {
        this.output.beginTask("locate address to place kernel");
        errdefer this.output.failed();

        var found = false;
        var mmap_it = this.mmap.iterator();
        while (mmap_it.next()) |mem| {
            if (mem.type == .conventional_memory) {
                if (mem.physical_start >= this.kernel_paddr) {
                    this.kernel_paddr = mem.physical_start;
                    found = true;
                }
            }
        }

        if (!found) {
            return error.NotFound;
        }

        this.output.ok();
    }

    fn loadSegments(this: *Loader) !void {
        this.output.beginTask("enumerate segments");
        errdefer this.output.failed();

        const kernel = this.kernel.?;
        const kernel_buf = this.kernel_buf.?;

        var ph_it = this.elf_header.iterateProgramHeadersBuffer(kernel_buf);
        var first_segment = true;
        var vaddr_offset: u64 = 0;

        this.output.ok();

        while (try ph_it.next()) |segment| {
            if (segment.p_type == elf.PT_LOAD) {
                this.output.beginTask("load segment");

                if (first_segment) {
                    this.kernel_vaddr = segment.p_vaddr;
                    first_segment = false;
                    vaddr_offset = this.kernel_vaddr -% this.kernel_paddr;
                }

                const vaddr: u64 = segment.p_vaddr +% vaddr_offset;

                if (vaddr & 0xFFF != 0) return error.SegmentUnaligned;
                if (segment.p_filesz > segment.p_memsz) return error.SegmentUnfit;

                const pages = try this.boot.allocatePages(
                    .{ .address = @ptrFromInt(vaddr) },
                    .loader_data,
                    segment.p_memsz >> 12,
                );
                errdefer this.boot.freePages(pages) catch {};

                const buf = flatten(pages);

                try kernel.setPosition(segment.p_offset);
                _ = try kernel.read(buf[0..segment.p_filesz]);

                const zero_start: u64 = vaddr + segment.p_filesz;
                const zero_count: u64 = segment.p_memsz - segment.p_filesz;

                if (zero_count > 0) {
                    @memset(@as([*]u8, @ptrFromInt(zero_start))[0..zero_count], 0);
                }

                this.output.ok();
                this.output.console.printf("    @{X}[{}]", .{ segment.p_vaddr, segment.p_memsz });
            } else {
                this.output.beginTask("skip segment");
                this.output.ok();
            }
        }

        if (first_segment) {
            return error.ELFNoCode;
        } else {
            return error.KernelFound;
        }
    }

    fn mmapInfo(this: *Loader) !void {
        this.output.beginTask("retrieve mmap info from EFI");
        errdefer this.output.failed();

        this.mmap_info = try this.boot.getMemoryMapInfo();
        this.output.ok();
    }

    fn mmapLoad(this: *Loader) !void {
        this.output.beginTask("load mmap");
        errdefer this.output.failed();

        const buf = flatten(this.mmap_pages.?);

        this.mmap = try this.boot.getMemoryMap(@alignCast(buf));
        this.output.ok();
    }

    fn mmapPages(this: *Loader) !void {
        this.output.beginTask("allocate pages for mmap");
        errdefer this.output.failed();

        const pages = pageLength(this.mmapSize());

        this.mmap_pages = try this.boot.allocatePages(.any, .loader_data, pages);
        this.output.ok();
    }

    fn mmapSize(this: *Loader) usize {
        return this.mmap_info.descriptor_size * this.mmap_info.len;
    }

    fn openKernel(this: *Loader) !void {
        this.output.beginTask("open kernel from ESP:" ++ KERNEL_PATH);
        errdefer this.output.failed();

        const kernal_path = unicode.utf8ToUtf16LeStringLiteral(KERNEL_PATH);

        this.kernel = try this.esp.?.open(kernal_path, .read, .{ .read_only = true });
        this.output.ok();
    }

    fn phSize(this: *Loader) usize {
        return std.math.mulWide(u16, this.elf_header.phnum, this.elf_header.phentsize);
    }

    fn prepareForProgramHeaders(this: *Loader) !void {
        this.output.beginTask("prepare buffer for kernel program headers");
        errdefer this.output.failed();

        const ph_size = this.phSize();
        const header_size = this.elf_header.phoff + ph_size;

        this.kernel_buf = try this.allocator.alloc(u8, header_size);
        this.output.ok();
    }

    fn readELFHeader(this: *Loader) !void {
        this.output.beginTask("read ELF header from kernel");
        errdefer this.output.failed();

        var header: elf.Elf64_Ehdr = undefined;
        const ptr: [*]u8 = @ptrCast(&header);
        const slice: []u8 = ptr[0..@sizeOf(elf.Elf64_Ehdr)];
        const kernel = this.kernel.?;

        try kernel.setPosition(0);
        _ = try kernel.read(slice);

        var reader: std.Io.Reader = .fixed(slice);

        this.elf_header = try elf.Header.read(&reader);
        this.output.ok();
    }

    fn readProgramHeaders(this: *Loader) !void {
        this.output.beginTask("read program headers from kernel");
        errdefer this.output.failed();

        const kernel = this.kernel.?;
        const kernel_buf = this.kernel_buf.?;

        try kernel.setPosition(0);
        _ = try kernel.read(kernel_buf);

        this.output.ok();
    }
};

/// Structured logging for boot loader.
const Logger = struct {
    console: Console,
    section_empty: bool = false,
    task_started: bool = false,

    pub fn init(console: Console) Logger {
        return .{ .console = console };
    }

    pub fn beginSection(this: *Logger, title: []const u8) void {
        this.flushSection();
        this.console.printf("{s}:\r\n", .{title});
        this.section_empty = true;
    }

    pub fn beginTask(this: *Logger, title: []const u8) void {
        this.flushTask();
        this.console.printf("  - {s}...", .{title});
        this.section_empty = false;
        this.task_started = true;
    }

    pub fn failed(this: *Logger) void {
        this.console.printf("failed\r\n", .{});
        this.task_started = false;
    }

    pub fn flushSection(this: *Logger) void {
        this.flushTask();

        if (this.section_empty) {
            this.console.printf("  - nothing to do\r\n", .{});
            this.section_empty = false;
        }
    }

    pub fn flushTask(this: *Logger) void {
        if (this.task_started) {
            this.console.printf("no status\r\n", .{});
            this.task_started = false;
        }
    }

    pub fn ok(this: *Logger) void {
        this.console.printf("ok\r\n", .{});
        this.task_started = false;
    }
};

/// Flatten a slice of pages into a slice of bytes.
fn flatten(pages: [][EFI_PAGE_SIZE]u8) []u8 {
    var flat: []u8 = &.{};

    flat.ptr = @ptrCast(pages.ptr);
    flat.len = pages.len * EFI_PAGE_SIZE;

    return flat;
}

/// Return number of pages required for a number of bytes.
fn pageLength(byteLength: usize) usize {
    return (byteLength + EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE;
}
