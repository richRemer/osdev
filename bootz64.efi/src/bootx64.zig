const std = @import("std");
const efi = @import("efi.zig");
const unicode = std.unicode;
const uefi = std.os.uefi;
const SimpleFileSystem = uefi.protocol.SimpleFileSystem;
const Allocator = std.mem.Allocator;

pub fn main() void {
    load() catch |err| @panic(@errorName(err));
    @panic("loader returned unexpectedly");
}

fn load() !void {
    const boot = uefi.system_table.boot_services.?;
    const disk = (try boot.locateProtocol(SimpleFileSystem, null)).?;
    const esp = try disk.openVolume();
    const kernel_path = unicode.utf8ToUtf16LeStringLiteral("\\kernel.bin");
    const kernel_file = try esp.open(kernel_path, .read, .{ .read_only = true });

    _ = kernel_file;

    return error.KernelFound;
}
