/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2016.
 */

#ifndef __ASM_ARCH_TRIX_MACH_MCOMM_H__
#define __ASM_ARCH_TRIX_MACH_MCOMM_H__

/*
 * @brief it's a wrapper used to send frame data to MCU side, i.e. for mcu_comm driver use. This is a synchronous call. Note that standby response interrupt is servered inside linux kernel.
 * @param[in]	data	pointer of mcu frame data
 * @param[in]	len	bytes of data
 *
 * @return
 * 	0 	- on success
 * 	<0	- error code, could be one of
 * 		EINVAL	- invalid paramters
 * 		ETIME	- timeout
 * 		EINTR	- interrupted by signal
 */
extern int trix_mcomm_send_data(void *data, unsigned int len);

#endif /*__ASM_ARCH_TRIX_MACH_MCOMM_H__*/
