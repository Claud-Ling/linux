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
 * %filename:     tmFlags.h %
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

 #ifndef TMFLAGS_H                                    
 #define TMFLAGS_H                                                   

 #define TMFL_OS_IS_NULLOS             1
 #define TMFL_ENDIAN                   0
 #define TMFL_ENDIAN_BIG               1
 #define TMFL_ENDIAN_LITTLE            0
 #define TMFL_REL_TRACE                0x00000003
 #define TMFL_REL_ASSERT               0x00000002
 #define TMFL_REL_DEBUG                0x00000001
 #define TMFL_REL_RETAIL               0x00000000

#define TMFL_REL  TMFL_REL_RETAIL

 #endif   /* TMFLAGS_H*/
