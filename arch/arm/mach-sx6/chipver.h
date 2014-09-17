#ifndef __CHIP_VER_H__
#define __CHIP_VER_H__

#define DEFINE_CHIP_VERSIONS(x)			\
	CHIP_##x##_REVA,			\
	CHIP_##x##_REVB,			\
	CHIP_##x##_REVC,			\
	CHIP_##x##_NEW = CHIP_##x##_REVA + 7,

/* 'x' specifies chip name, in upper case, i.e. SX6, SX7, .etc */
#define CHIP_REVA(x) CHIP_##x##_REVA
#define CHIP_REVB(x) CHIP_##x##_REVB
#define CHIP_REVC(x) CHIP_##x##_REVC
#define CHIP_NEW(x) CHIP_##x##_NEW

enum chip_version{
	DEFINE_CHIP_VERSIONS(SX6)
	DEFINE_CHIP_VERSIONS(SX7)

	CHIP_UNKNOWN  = 0xffffffff,
};

#undef DEFINE_CHIP_VERSIONS
#ifndef __ASSEMBLY__
extern int dtv_get_chip_id(void);
extern const char* dtv_chip_name(int chip_id);

#define dtv_chip_is_stub(id, stub)	\
		(((id) & 0x7) == CHIP_REVA(stub))

#define dtv_chip_is_sx6(id) dtv_chip_is_stub(id, SX6)
#define dtv_chip_is_sx7(id) dtv_chip_is_stub(id, SX7)
 
#endif /*__ASSEMBLY__*/

#endif /*__CHIP_VER_H__*/
