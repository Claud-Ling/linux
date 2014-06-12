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
 * %filename:     tmNxTypes.h %
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

#ifndef TMNXTYPES_H
#define TMNXTYPES_H

/*-----------------------------------------------------------------------------
** Standard include files:
**-----------------------------------------------------------------------------
*/

#include "tmFlags.h"          

#ifdef __cplusplus
extern "C"
{
#endif

/*-----------------------------------------------------------------------------
** Types and defines:
**-----------------------------------------------------------------------------
*/

#define TM_FALSE            0       /* replaces legacy False macro */
#define TM_TRUE             1       /* replaces legacy True  macro */

#define TM_ENDIAN_BIG       0       /* replaces legacy BigEndian    macro */
#define TM_ENDIAN_LITTLE    1       /* replaces legacy LittleEndian macro */

#ifndef False
#define False         (TM_FALSE)  /* NOTE: Legacy - use TM_FALSE instead */
#endif

#ifndef True
#define True          (TM_TRUE)   /* NOTE: Legacy - use TM_TRUE  instead */
#endif

#ifdef __cplusplus
#define Null          0
#else
#define Null          ((Void *) 0)
#endif

#define HAL_DEVICE_NAME_LENGTH  16

/*
** Standard Types
*/
typedef void            Void;       /* Void (typeless) */
typedef signed   char   Int8;       /*  8-bit   signed integer */
typedef signed   short  Int16;      /* 16-bit   signed integer */
typedef signed   long   Int32;      /* 32-bit   signed integer */
typedef unsigned char   UInt8;      /*  8-bit unsigned integer */
typedef unsigned short  UInt16;     /* 16-bit unsigned integer */
typedef unsigned long   UInt32;     /* 32-bit unsigned integer */
typedef float           Float;      /* 32-bit floating point */
typedef unsigned int    Bool;       /* Boolean (True/False) */
typedef char            Char;       /* character, character array ptr */
typedef int             Int;        /* machine-natural integer */
typedef unsigned int    UInt;       /* machine-natural unsigned integer */
typedef char           *String;     /* Null-terminated 8-bit char str */


typedef struct tmVersion
{
    UInt8   majorVersion;
    UInt8   minorVersion;
    UInt16  buildVersion;
}   tmVersion_t, *ptmVersion_t;


typedef signed   int    IBits32;    /* 32-bit   signed integer bitfields */
typedef unsigned int    UBits32;    /* 32-bit unsigned integer bitfields */

typedef Int8    *pInt8;             /*  8-bit   signed integer ptr */
typedef Int16   *pInt16;            /* 16-bit   signed integer ptr */
typedef Int32   *pInt32;            /* 32-bit   signed integer ptr */
typedef IBits32 *pIBits32;          /* 32-bit   signed integer bitfield ptr */
typedef UBits32 *pUBits32;          /* 32-bit unsigned integer bitfield ptr */
typedef UInt8   *pUInt8;            /*  8-bit unsigned integer ptr */
typedef UInt16  *pUInt16;           /* 16-bit unsigned integer ptr */
typedef UInt32  *pUInt32;           /* 32-bit unsigned integer ptr */
typedef Void    *pVoid;             /* Void (typeless) ptr */
typedef Float   *pFloat;            /* 32-bit floating point, float ptr */
typedef double  Double, *pDouble;   /* 32/64-bit floating point, double ptr */
typedef Bool    *pBool;             /* Boolean (True/False) ptr */
typedef Char    *pChar;             /* character, character array ptr */
typedef Int     *pInt;              /* machine-natural integer ptr */
typedef UInt    *pUInt;             /* machine-natural unsigned integer ptr */
typedef String  *pString;           /* Null-terminated 8-bit char str ptr */



typedef UInt32 tmErrorCode_t;


/*-----------------------------------------------------------------------------
** Hardware device power states
*/
typedef enum tmPowerState
{
    tmPowerOn,                          /* Device powered on      (D0 state) */
    tmPowerStandby,                     /* Device power standby   (D1 state) */
    tmPowerSuspend,                     /* Device power suspended (D2 state) */
    tmPowerOff                          /* Device powered off     (D3 state) */
}   tmPowerState_t, *ptmPowerState_t;

/*-----------------------------------------------------------------------------
** Software Version Structure
*/
typedef struct tmSWVersion
{
    UInt32      compatibilityNr;        /* Interface compatibility number */
    UInt32      majorVersionNr;         /* Interface major version number */
    UInt32      minorVersionNr;         /* Interface minor version number */

}   tmSWVersion_t, *ptmSWVersion_t;

typedef Int tmInstance_t, *ptmInstance_t;

/*-----------------------------------------------------------------------------

** HW Unit Selection
*/

typedef Int tmUnitSelect_t, *ptmUnitSelect_t;

#define tmUnitNone (-1)
#define tmUnit0    0
#define tmUnit1    1
#define tmUnit2    2
#define tmUnit3    3
#define tmUnit4    4
#define tmUnit5    5
#define tmUnit6    6
#define tmUnit7    7

#ifdef __cplusplus
}
#endif


#endif /* ifndef TMNXTYPES_H */

