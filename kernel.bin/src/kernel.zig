const uart = @import("uart.zig");
const ansi = @import("ansi.zig");

// https://wiki.osdev.org/Zig_Bare_Bones

var stack: [0x4000]u8 align(16) linksection(".bss") = undefined;

export fn kenter() callconv(.naked) noreturn {
    asm volatile (
        \\ movl %[stack_ptr], %%esp
        \\ movl %%esp, %%ebp
        \\ call %[kmain:P]
        :
        : [stack_ptr] "i" (&@as([*]align(16) u8, @ptrCast(&stack))[stack.len]),
          [kmain] "X" (&kmain),
    );
}

noinline fn kmain() callconv(.c) noreturn {
    uart.initialize();
    uart.puts(ansi.clrscr);
    uart.puts("kernel has taken over\n");

    while (true) {
        asm volatile ("hlt");
    }
}
