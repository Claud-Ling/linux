/*
 * s2ramctrl.h
 * This file defines struct s2ram_resume_frame
 */

#ifndef __S2RAMCTRL_H__
#define __S2RAMCTRL_H__

#define S2RAM_FRAME_SIZE 	32	/* sizeof struct s2ram_resume_frame */

#ifndef __ASSEMBLY__

/*
 * s2ram resume frame structure.
 * This structure defines the way the resume entry
 * and other stuffs are stored during an s2ram operation
 */
struct s2ram_resume_frame{
	long data[(S2RAM_FRAME_SIZE) >> 2];
};

#define S2RAM_ENTRY	data[0]		/* resume entry, keep at the first!! */

#endif //__ASSEMBLY__

#endif
