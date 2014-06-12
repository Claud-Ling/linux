#ifndef __CHIP_VER_H__
#define __CHIP_VER_H__

#define CHIP_SX6_REVA	0x00000000
#define CHIP_SX6_REVB	0x00000001
#define CHIP_SX6_NEW	0x00000009

#define CHIP_UNKNOWN	0xffffffff

#ifndef __ASSEMBLY__
extern int dtv_get_chip_id(void);
extern const char* dtv_chip_name(int chip_id);
#endif /*__ASSEMBLY__*/

#endif /*__CHIP_VER_H__*/
