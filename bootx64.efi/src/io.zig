const std = @import("std");
const config = @import("config.zig");
const uefi = std.os.uefi;
const BootServices = uefi.tables.BootServices;
const SimpleTextInput = uefi.protocol.SimpleTextInput;
const SimpleTextOutput = uefi.protocol.SimpleTextOutput;

/// This value should be increased if receiving NoSpaceLeft error from printf
pub const max_printf_size = 1024;

pub const Console = struct {
    in: *SimpleTextInput,
    out: *SimpleTextOutput,

    pub fn init(in: *SimpleTextInput, out: *SimpleTextOutput) Console {
        out.reset(false) catch {};
        return .{ .in = in, .out = out };
    }

    pub fn dbg(this: Console, comptime fmt: []const u8, args: anytype) void {
        if (config.debug) this.printf("BOOTDBG:" ++ fmt ++ "\r\n", args);
    }

    pub fn err(this: Console, comptime fmt: []const u8, args: anytype) void {
        this.printf("BOOTERR:" ++ fmt ++ "\r\n", args);
    }

    pub fn log(this: Console, comptime fmt: []const u8, args: anytype) void {
        this.printf("BOOTLOG:" ++ fmt ++ "\r\n", args);
    }

    pub fn puts(this: Console, string: []const u8) void {
        for (string) |ch| {
            const ch_str = [1:0]u16{ch};
            _ = this.out.outputString(&ch_str) catch {};
        }
    }

    pub fn printf(this: Console, comptime fmt: []const u8, args: anytype) void {
        var buffer: [max_printf_size]u8 = undefined;
        const string = std.fmt.bufPrint(&buffer, fmt, args) catch unreachable;
        this.puts(string);
    }
};
