pub const MAGIC = 0x1BADB002;

pub const Header = packed struct {
    magic: u32 = MAGIC,
    flags: Flags,
    checksum: u32,
    padding: u32 = 0,

    pub const Flags = packed struct(u32) {
        page_align: bool = false,
        mem_info: bool = false,
        video_mode: bool = false,
        reserved: u13 = 0,
        aout_kludge: bool = false,
        padding: u15 = 0,
    };

    pub fn init(flags: Flags) Header {
        return Header{
            .flags = flags,
            .checksum = ~(MAGIC + @as(u32, @bitCast(flags))) + 1,
        };
    }
};
