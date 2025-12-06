const uart = @import("uart.zig");
const ansi = @import("ansi.zig");
const mb = @import("multiboot.zig");
const config = @import("kconfig.zon");
const StackSize = config.StackSize;
const StackAlign = config.StackAlign;

// https://wiki.osdev.org/Zig_Bare_Bones

export var multiboot: mb.Header align(4) linksection(".multiboot") = mb.Header.init(.{
    .page_align = true,
    .mem_info = true,
});

var stack: [StackSize]u8 align(StackAlign) linksection(".bss") = undefined;

export fn _start() callconv(.naked) noreturn {
    asm volatile (
        \\ movl %[stack_ptr], %%esp
        \\ movl %%esp, %%ebp
        \\ call %[kmain:P]
        :
        : [stack_ptr] "i" (&@as([*]align(StackAlign) u8, @ptrCast(&stack))[stack.len]),
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
