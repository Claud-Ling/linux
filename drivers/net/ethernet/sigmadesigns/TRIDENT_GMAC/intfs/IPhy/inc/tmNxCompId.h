/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (c) 2006-2007, LIPP Alliance
 * All Rights Reserved.
 *
 *---------------------------------------------------------------------------
 * %filename:     tmNxCompId.h %
 * %pid_version:          %
 *---------------------------------------------------------------------------
 * DESCRIPTION:  
 *
 * DOCUMENT REF: 
 *
 * NOTES:        None
 *
 *-----------------------------------------------------------------------------
 *
*/

#ifndef TMNXCOMPID_H
#define TMNXCOMPID_H

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Standard include files:                                                  */
/*                                                                            */
/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif

#define CID_IID_FLAG_BITSHIFT    31
#define CID_ID_BITSHIFT          12

#define TM_OK                     0U         /* Global success return status   */

#define CID_ID(number)           ((number) << CID_ID_BITSHIFT)
#define CID_FLAG                 (0x1U << CID_IID_FLAG_BITSHIFT)


#define CID_BSL_PHY                 (CID_ID(0x80deU) | CID_FLAG)

#define TM_ERR_INIT_FAILED              0x014U /* Initialization failed        */
#define TM_ERR_BAD_UNIT_NUMBER          0x005U /* Invalid device unit number   */
#define TM_ERR_NOT_SUPPORTED            0x013U /* Function is not supported    */
#define TM_ERR_TIMEOUT                  0x01FU /* Timeout error                */
#define TM_ERR_HW_RESET_FAILED          0x04BU /* Hardware reset failed        */
#define TM_ERR_READ                     0x017U /* Read error                   */
#define TM_ERR_WRITE                    0x018U /* Write error                  */

#ifdef __cplusplus
}
#endif

#endif /* TMNXCOMPID_H ----------------- */
