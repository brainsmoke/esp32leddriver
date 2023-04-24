
DEVICE = PMS150C

FUSE_ADDRESS = 0x3ff
FUSE_BITS = 0xffd

FUSE_SET_SECURITY_ON = (FUSE_BITS)
FUSE_SET_SECURITY    = (FUSE_BITS & ~1)
FUSE_SET_LVR_4V      = (FUSE_BITS & ~(7 << 2))
FUSE_SET_LVR_3V5     = (FUSE_SET_LVR_4V | (1 << 2))
FUSE_SET_LVR_3V      = (FUSE_SET_LVR_4V | (2 << 2))
FUSE_SET_LVR_2V75    = (FUSE_SET_LVR_4V | (3 << 2))
FUSE_SET_LVR_2V5     = (FUSE_SET_LVR_4V | (4 << 2))
FUSE_SET_LVR_2V2     = (FUSE_SET_LVR_4V | (6 << 2))
FUSE_SET_LVR_2V      = (FUSE_SET_LVR_4V | (7 << 2)) ; default
FUSE_SET_LVR_1V8     = (FUSE_SET_LVR_4V | (5 << 2))

FUSE_SET_IO_DRV_HIGH = (FUSE_BITS)
FUSE_SET_IO_DRV_LOW  = (FUSE_BITS & ~(1<<7))

FUSE_BOOTUP_FAST     = (FUSE_BITS)
FUSE_BOOTUP_SLOW     = (FUSE_BITS & ~(3 << 10))

; see PMS150 datasheet 5.10.3
.macro brownout_erratum_workaround_init
	set1 inten, #7
	mov a, #0
	mov intrq, a
.endm

.macro brownout_erratum_workaround_2cycles
	t1sn inten, #7
	reset
.endm

