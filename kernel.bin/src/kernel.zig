const uart = @import("uart.zig");
const ansi = @import("ansi.zig");

export fn kmain() void {
    uart.initialize();
    uart.puts(ansi.clrscr);
    uart.puts("kernel has taken over\n");
    while (true) {}
}
