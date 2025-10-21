const std = @import("std");

pub fn build(b: *std.Build) void {
    const exe = b.addExecutable(.{
        .name = "kernel.bin",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/kernel.zig"),
            .target = b.resolveTargetQuery(std.Target.Query{
                .cpu_arch = .x86_64,
                .os_tag = .freestanding,
                .abi = .none,
                .ofmt = .elf,
            }),
            .optimize = b.standardOptimizeOption(.{}),
        }),
        .use_lld = true,
        .use_llvm = true,
    });

    exe.entry = .disabled;
    exe.setLinkerScript(b.path("src/kernel.ld"));

    b.installArtifact(exe);
}
