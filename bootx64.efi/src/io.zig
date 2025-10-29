const std = @import("std");
const config = @import("config.zig");
const uefi = std.os.uefi;
const BootServices = uefi.tables.BootServices;
const SimpleTextInput = uefi.protocol.SimpleTextInput;
const SimpleTextOutput = uefi.protocol.SimpleTextOutput;

/// This value should be increased if receiving NoSpaceLeft error from printf
pub const printf_buffer_size = 1024;

/// Handle writing text to the console.
pub const Console = struct {
    in: *SimpleTextInput,
    out: *SimpleTextOutput,

    /// Initialize with the input/output consoles from the UEFI system table.
    pub fn init(in: *SimpleTextInput, out: *SimpleTextOutput) Console {
        out.reset(false) catch {};
        return .{ .in = in, .out = out };
    }

    /// Write a formatted message, but only if config.debug is true.
    pub fn dbg(this: Console, comptime fmt: []const u8, args: anytype) void {
        if (config.debug) this.printf(fmt ++ "\r\n", args);
    }

    /// Write a formatted error message.
    pub fn err(this: Console, comptime fmt: []const u8, args: anytype) void {
        this.printf("ERROR:" ++ fmt ++ "\r\n", args);
    }

    /// Write a formatted message.
    pub fn log(this: Console, comptime fmt: []const u8, args: anytype) void {
        this.printf(fmt ++ "\r\n", args);
    }

    /// Write string.
    pub fn puts(this: Console, string: []const u8) void {
        for (string) |ch| {
            const ch_str = [1:0]u16{ch};
            _ = this.out.outputString(&ch_str) catch {};
        }
    }

    /// Write a formatted string.
    pub fn printf(this: Console, comptime fmt: []const u8, args: anytype) void {
        var buffer: [printf_buffer_size]u8 = undefined;
        const string = std.fmt.bufPrint(&buffer, fmt, args) catch unreachable;
        this.puts(string);
    }
};
