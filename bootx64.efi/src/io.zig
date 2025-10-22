const std = @import("std");
const uefi = std.os.uefi;
const BootServices = uefi.tables.BootServices;
const SimpleTextInput = uefi.protocol.SimpleTextInput;
const SimpleTextOutput = uefi.protocol.SimpleTextOutput;

pub const Console = struct {
    in: *SimpleTextInput,
    out: *SimpleTextOutput,

    pub fn init(in: *SimpleTextInput, out: *SimpleTextOutput) Console {
        out.reset(false) catch {};
        return .{ .in = in, .out = out };
    }

    pub fn puts(this: Console, string: []const u8) void {
        for (string) |ch| {
            const ch_str = [1:0]u16{ch};
            _ = this.out.outputString(&ch_str) catch {};
        }
    }

    pub fn printf(this: Console, comptime fmt: []const u8, args: anytype) void {
        var buffer: [256]u8 = undefined;
        const string = std.fmt.bufPrint(&buffer, fmt, args) catch unreachable;
        this.puts(string);
    }
};
