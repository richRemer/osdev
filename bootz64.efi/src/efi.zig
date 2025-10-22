const std = @import("std");
const uefi = std.os.uefi;
const BootServices = uefi.tables.BootServices;

pub fn locate(Protocol: type, boot: *BootServices) !*Protocol {
    const result = boot.locateProtocol(Protocol, null) catch null;
    return result orelse error.ProtocolNotFound;
}
