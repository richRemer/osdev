const Loader = @import("boot.zig").Loader;

/// Boot loader entry point.
pub fn main() void {
    var loader = Loader.init();
    defer loader.deinit();

    loader.load() catch |err| @panic(@errorName(err));
    @panic("loader returned unexpectedly");
}
