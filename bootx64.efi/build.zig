const std = @import("std");

pub fn build(b: *std.Build) void {
    b.installArtifact(b.addExecutable(.{
        .name = "bootx64",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/bootx64.zig"),
            .target = b.resolveTargetQuery(std.Target.Query{
                .cpu_arch = .x86_64,
                .os_tag = .uefi,
                .abi = .msvc,
                .ofmt = .coff,
            }),
            .optimize = b.standardOptimizeOption(.{}),
        }),
    }));
}
