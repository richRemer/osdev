const io_port = @import("io_port.zig");

pub const COM1 = 0x3f8;

pub fn initialize() void {
    // zero out Interrupt Enable
    io_port.outb(COM1 + 1, 0x00);

    // set DLAB bit in Line Control
    io_port.outb(COM1 + 3, 0x80);

    // DLAB out baud rate lsb/msb
    io_port.outb(COM1 + 0, 0x03);
    io_port.outb(COM1 + 1, 0x00);

    // set Line Control to default (8-bit, no parity, 1 stop bit)
    io_port.outb(COM1 + 3, 0x03);

    // set FIFO control
    io_port.outb(COM1 + 2, 0xc7);

    // set modem control
    io_port.outb(COM1 + 4, 0x0b);
}

pub inline fn putch(ch: u8) void {
    while (!ready()) {}
    io_port.outb(COM1, ch);
}

pub fn puts(str: []const u8) void {
    for (str) |ch| {
        putch(ch);
    }
}

inline fn ready() bool {
    // check Line Status register
    return (io_port.inb(COM1 + 5) & 0x20) != 0;
}
