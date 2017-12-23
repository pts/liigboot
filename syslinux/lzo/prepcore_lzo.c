#ifndef __LZO_CONF_H
#define __LZO_CONF_H

#if !defined(__LZO_IN_MINILZO)
#if defined(LZO_CFG_FREESTANDING)
#  define LZO_LIBC_FREESTANDING 1
#  define LZO_OS_FREESTANDING 1
#  define ACC_LIBC_FREESTANDING 1
#  define ACC_OS_FREESTANDING 1
#endif
#if defined(LZO_CFG_NO_UNALIGNED)
#  define ACC_CFG_NO_UNALIGNED 1
#endif
#if defined(LZO_ARCH_GENERIC)
#  define ACC_ARCH_GENERIC 1
#endif
#if defined(LZO_ABI_NEUTRAL_ENDIAN)
#  define ACC_ABI_NEUTRAL_ENDIAN 1
#endif
#if defined(LZO_HAVE_CONFIG_H)
#  define ACC_CONFIG_NO_HEADER 1
#endif
#if defined(LZO_CFG_EXTRA_CONFIG_HEADER)
#  include LZO_CFG_EXTRA_CONFIG_HEADER
#endif
#if defined(__LZOCONF_H) || defined(__LZOCONF_H_INCLUDED)
#  error "include this file first"
#endif
#include "prepcore_lzo.h"
/*#include "lzo/lzoconf.h"*/
#endif

#if (LZO_VERSION < 0x02000) || !defined(__LZOCONF_H_INCLUDED)
#  error "version mismatch"
#endif


/***********************************************************************
// pragmas
************************************************************************/

#if (LZO_CC_BORLANDC && LZO_ARCH_I086)
#  pragma option -h         /* enable fast huge pointers */
#endif

#if (LZO_CC_MSC && (_MSC_VER >= 1000))
#  pragma warning(disable: 4127 4701)
#endif
#if (LZO_CC_MSC && (_MSC_VER >= 1300))
   /* avoid `-Wall' warnings in system header files */
#  pragma warning(disable: 4820)
   /* avoid warnings about inlining */
#  pragma warning(disable: 4514 4710 4711)
#endif

#if (LZO_CC_SUNPROC)
#  pragma error_messages(off,E_END_OF_LOOP_CODE_NOT_REACHED)
#  pragma error_messages(off,E_LOOP_NOT_ENTERED_AT_TOP)
#endif


/***********************************************************************
//
************************************************************************/

#if defined(__LZO_MMODEL_HUGE) && (!LZO_HAVE_MM_HUGE_PTR)
#  error "this should not happen - check defines for __huge"
#endif

#if defined(__LZO_IN_MINILZO) || defined(LZO_CFG_FREESTANDING)
#elif (LZO_OS_DOS16 || LZO_OS_OS216 || LZO_OS_WIN16)
#  define ACC_WANT_ACC_INCD_H 1
#  define ACC_WANT_ACC_INCE_H 1
#  define ACC_WANT_ACC_INCI_H 1
#elif 1
#  include <string.h>
#else
#  define ACC_WANT_ACC_INCD_H 1
#endif
#include "prepcore_lzo_miniacc.h"  /* TODO(pts): Remove this? Do we need all this? */


#if defined(LZO_CFG_FREESTANDING)
#  undef HAVE_MEMCMP
#  undef HAVE_MEMCPY
#  undef HAVE_MEMMOVE
#  undef HAVE_MEMSET
#endif

#if !defined(HAVE_MEMCMP)
#  undef memcmp
#  define memcmp(a,b,c)         lzo_memcmp(a,b,c)
#elif !defined(__LZO_MMODEL_HUGE)
#  define lzo_memcmp(a,b,c)     memcmp(a,b,c)
#endif
#if !defined(HAVE_MEMCPY)
#  undef memcpy
#  define memcpy(a,b,c)         lzo_memcpy(a,b,c)
#elif !defined(__LZO_MMODEL_HUGE)
#  define lzo_memcpy(a,b,c)     memcpy(a,b,c)
#endif
#if !defined(HAVE_MEMMOVE)
#  undef memmove
#  define memmove(a,b,c)        lzo_memmove(a,b,c)
#elif !defined(__LZO_MMODEL_HUGE)
#  define lzo_memmove(a,b,c)    memmove(a,b,c)
#endif
#if !defined(HAVE_MEMSET)
#  undef memset
#  define memset(a,b,c)         lzo_memset(a,b,c)
#elif !defined(__LZO_MMODEL_HUGE)
#  define lzo_memset(a,b,c)     memset(a,b,c)
#endif


#undef NDEBUG
#if defined(LZO_CFG_FREESTANDING)
#  undef LZO_DEBUG
#  define NDEBUG 1
#  undef assert
#  define assert(e) ((void)0)
#else
#  if !defined(LZO_DEBUG)
#    define NDEBUG 1
#  endif
#  include <assert.h>
#endif

#if 0 && defined(__BOUNDS_CHECKING_ON)
#  include <unchecked.h>
#else
#  define BOUNDS_CHECKING_OFF_DURING(stmt)      stmt
#  define BOUNDS_CHECKING_OFF_IN_EXPR(expr)     (expr)
#endif

#if !defined(__lzo_inline)
#  define __lzo_inline
#endif
#if !defined(__lzo_forceinline)
#  define __lzo_forceinline
#endif
#if !defined(__lzo_noinline)
#  define __lzo_noinline
#endif


/***********************************************************************
//
************************************************************************/

#if 1
#  define LZO_BYTE(x)       ((unsigned char) (x))
#else
#  define LZO_BYTE(x)       ((unsigned char) ((x) & 0xff))
#endif

#define LZO_MAX(a,b)        ((a) >= (b) ? (a) : (b))
#define LZO_MIN(a,b)        ((a) <= (b) ? (a) : (b))
#define LZO_MAX3(a,b,c)     ((a) >= (b) ? LZO_MAX(a,c) : LZO_MAX(b,c))
#define LZO_MIN3(a,b,c)     ((a) <= (b) ? LZO_MIN(a,c) : LZO_MIN(b,c))

#define lzo_sizeof(type)    ((lzo_uint) (sizeof(type)))

#define LZO_HIGH(array)     ((lzo_uint) (sizeof(array)/sizeof(*(array))))

/* this always fits into 16 bits */
#define LZO_SIZE(bits)      (1u << (bits))
#define LZO_MASK(bits)      (LZO_SIZE(bits) - 1)

#define LZO_LSIZE(bits)     (1ul << (bits))
#define LZO_LMASK(bits)     (LZO_LSIZE(bits) - 1)

#define LZO_USIZE(bits)     ((lzo_uint) 1 << (bits))
#define LZO_UMASK(bits)     (LZO_USIZE(bits) - 1)

#if !defined(DMUL)
#if 0
   /* 32*32 multiplies may be faster than 64*64 on some 64-bit machines,
    * but then we need extra casts from unsigned<->size_t */
#  define DMUL(a,b) ((lzo_xint) ((lzo_uint32)(a) * (lzo_uint32)(b)))
#else
#  define DMUL(a,b) ((lzo_xint) ((a) * (b)))
#endif
#endif


/***********************************************************************
// compiler and architecture specific stuff
************************************************************************/

/* Some defines that indicate if memory can be accessed at unaligned
 * memory addresses. You should also test that this is actually faster
 * even if it is allowed by your system.
 */

#if 1 && !defined(LZO_CFG_NO_UNALIGNED)
#if 1 && (LZO_ARCH_AMD64 || LZO_ARCH_I386)
#  if (LZO_SIZEOF_SHORT == 2)
#    define LZO_UNALIGNED_OK_2
#  endif
#  if (LZO_SIZEOF_INT == 4)
#    define LZO_UNALIGNED_OK_4
#  endif
#endif
#endif

#if defined(LZO_UNALIGNED_OK_2)
  LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(short) == 2)
#endif
#if defined(LZO_UNALIGNED_OK_4)
  LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_uint32) == 4)
#elif defined(LZO_ALIGNED_OK_4)
  LZO_COMPILE_TIME_ASSERT_HEADER(sizeof(lzo_uint32) == 4)
#endif


/* Fast memcpy that copies multiples of 8 byte chunks.
 * len is the number of bytes.
 * note: all parameters must be lvalues, len >= 8
 *       dest and src advance, len is undefined afterwards
 */

#define MEMCPY8_DS(dest,src,len) \
    lzo_memcpy(dest,src,len); dest += len; src += len

#define BZERO8_PTR(s,l,n) \
    lzo_memset((lzo_voidp)(s),0,(lzo_uint)(l)*(n))

#define MEMCPY_DS(dest,src,len) \
    do *dest++ = *src++; while (--len > 0)


/***********************************************************************
// some globals
************************************************************************/

__LZO_EXTERN_C int __lzo_init_done;
__LZO_EXTERN_C const char __lzo_copyright[];
LZO_EXTERN(const lzo_bytep) lzo_copyright(void);


/***********************************************************************
//
************************************************************************/

/* --- lzo_ptr.h */

/* lzo_ptr.h -- low-level pointer constructs

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_PTR_H
#define __LZO_PTR_H

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
// Integral types
************************************************************************/

#if !defined(lzo_uintptr_t)
#  if defined(__LZO_MMODEL_HUGE)
#    define lzo_uintptr_t   unsigned long
#  else
#    define lzo_uintptr_t   acc_uintptr_t
#    ifdef __ACC_INTPTR_T_IS_POINTER
#      define __LZO_UINTPTR_T_IS_POINTER 1
#    endif
#  endif
#endif


/***********************************************************************
//
************************************************************************/

/* Always use the safe (=integral) version for pointer-comparisions.
 * The compiler should optimize away the additional casts anyway.
 *
 * Note that this only works if the representation and ordering
 * of the pointer and the integral is the same (at bit level).
 *
 * Most 16-bit compilers have their own view about pointers -
 * fortunately they don't care about comparing pointers
 * that are pointing to Nirvana.
 */

#if (LZO_ARCH_I086)
#define PTR(a)              ((lzo_bytep) (a))
/* only need the low bits of the pointer -> offset is ok */
#define PTR_ALIGNED_4(a)    ((ACC_PTR_FP_OFF(a) & 3) == 0)
#define PTR_ALIGNED2_4(a,b) (((ACC_PTR_FP_OFF(a) | ACC_PTR_FP_OFF(b)) & 3) == 0)
#elif (LZO_MM_PVP)
#define PTR(a)              ((lzo_bytep) (a))
#define PTR_ALIGNED_8(a)    ((((lzo_uintptr_t)(a)) >> 61) == 0)
#define PTR_ALIGNED2_8(a,b) ((((lzo_uintptr_t)(a)|(lzo_uintptr_t)(b)) >> 61) == 0)
#else
#define PTR(a)              ((lzo_uintptr_t) (a))
#define PTR_LINEAR(a)       PTR(a)
#define PTR_ALIGNED_4(a)    ((PTR_LINEAR(a) & 3) == 0)
#define PTR_ALIGNED_8(a)    ((PTR_LINEAR(a) & 7) == 0)
#define PTR_ALIGNED2_4(a,b) (((PTR_LINEAR(a) | PTR_LINEAR(b)) & 3) == 0)
#define PTR_ALIGNED2_8(a,b) (((PTR_LINEAR(a) | PTR_LINEAR(b)) & 7) == 0)
#endif

#define PTR_LT(a,b)         (PTR(a) < PTR(b))
#define PTR_GE(a,b)         (PTR(a) >= PTR(b))
#define PTR_DIFF(a,b)       (PTR(a) - PTR(b))
#define pd(a,b)             ((lzo_uint) ((a)-(b)))


LZO_EXTERN(lzo_uintptr_t)
__lzo_ptr_linear(const lzo_voidp ptr);


typedef union
{
    char            a_char;
    unsigned char   a_uchar;
    short           a_short;
    unsigned short  a_ushort;
    int             a_int;
    unsigned int    a_uint;
    long            a_long;
    unsigned long   a_ulong;
    lzo_int         a_lzo_int;
    lzo_uint        a_lzo_uint;
    lzo_int32       a_lzo_int32;
    lzo_uint32      a_lzo_uint32;
    ptrdiff_t       a_ptrdiff_t;
    lzo_uintptr_t   a_lzo_uintptr_t;
    lzo_voidp       a_lzo_voidp;
    void *          a_void_p;
    lzo_bytep       a_lzo_bytep;
    lzo_bytepp      a_lzo_bytepp;
    lzo_uintp       a_lzo_uintp;
    lzo_uint *      a_lzo_uint_p;
    lzo_uint32p     a_lzo_uint32p;
    lzo_uint32 *    a_lzo_uint32_p;
    unsigned char * a_uchar_p;
    char *          a_char_p;
}
lzo_full_align_t;



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

/*
vi:ts=4:et
*/



/* --- */

/* Generate compressed data in a deterministic way.
 * This is fully portable, and compression can be faster as well.
 * A reason NOT to be deterministic is when the block size is
 * very small (e.g. 8kB) or the dictionary is big, because
 * then the initialization of the dictionary becomes a relevant
 * magnitude for compression speed.
 */
#define LZO_DETERMINISTIC


#define LZO_DICT_USE_PTR
#if 0 && (LZO_ARCH_I086)
#  undef LZO_DICT_USE_PTR
#endif

#if defined(LZO_DICT_USE_PTR)
#  define lzo_dict_t    const lzo_bytep
#  define lzo_dict_p    lzo_dict_t __LZO_MMODEL *
#else
#  define lzo_dict_t    lzo_uint
#  define lzo_dict_p    lzo_dict_t __LZO_MMODEL *
#endif


#endif /* already included */

/*
vi:ts=4:et
*/


#include "prepcore_lzo.h"

/* --- lzo_util.c */

/* lzo_util.c -- utilities for the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


/***********************************************************************
//
************************************************************************/

/* If you use the LZO library in a product, I would appreciate that you
 * keep this copyright string in the executable of your product.
.*/

const char __lzo_copyright[] =
#if !defined(__LZO_IN_MINLZO)
    /* save space as some people want a really small decompressor */
    LZO_VERSION_STRING;
#else
    "\r\n\n"
    "LZO data compression library.\n"
    "$Copyright: LZO (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Markus Franz Xaver Johannes Oberhumer\n"
    "<markus@oberhumer.com>\n"
    "http://www.oberhumer.com $\n\n"
    "$Id: LZO version: v" LZO_VERSION_STRING ", " LZO_VERSION_DATE " $\n"
    "$Built: " __DATE__ " " __TIME__ " $\n"
    "$Info: " LZO_INFO_STRING " $\n";
#endif


LZO_PUBLIC(const lzo_bytep)
lzo_copyright(void)
{
#if (LZO_OS_DOS16 && LZO_CC_TURBOC)
    return (lzo_voidp) __lzo_copyright;
#else
    return (const lzo_bytep) __lzo_copyright;
#endif
}

LZO_PUBLIC(unsigned)
lzo_version(void)
{
    return LZO_VERSION;
}

LZO_PUBLIC(const char *)
lzo_version_string(void)
{
    return LZO_VERSION_STRING;
}

LZO_PUBLIC(const char *)
lzo_version_date(void)
{
    return LZO_VERSION_DATE;
}

LZO_PUBLIC(const lzo_charp)
_lzo_version_string(void)
{
    return LZO_VERSION_STRING;
}

LZO_PUBLIC(const lzo_charp)
_lzo_version_date(void)
{
    return LZO_VERSION_DATE;
}


/***********************************************************************
// adler32 checksum
// adapted from free code by Mark Adler <madler@alumni.caltech.edu>
// see http://www.zlib.org/
************************************************************************/

#define LZO_BASE 65521u /* largest prime smaller than 65536 */
#define LZO_NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define LZO_DO1(buf,i)  s1 += buf[i]; s2 += s1
#define LZO_DO2(buf,i)  LZO_DO1(buf,i); LZO_DO1(buf,i+1);
#define LZO_DO4(buf,i)  LZO_DO2(buf,i); LZO_DO2(buf,i+2);
#define LZO_DO8(buf,i)  LZO_DO4(buf,i); LZO_DO4(buf,i+4);
#define LZO_DO16(buf,i) LZO_DO8(buf,i); LZO_DO8(buf,i+8);

LZO_PUBLIC(lzo_uint32)
lzo_adler32(lzo_uint32 adler, const lzo_bytep buf, lzo_uint len)
{
    lzo_uint32 s1 = adler & 0xffff;
    lzo_uint32 s2 = (adler >> 16) & 0xffff;
    unsigned k;

    if (buf == NULL)
        return 1;

    while (len > 0)
    {
        k = len < LZO_NMAX ? (unsigned) len : LZO_NMAX;
        len -= k;
        if (k >= 16) do
        {
            LZO_DO16(buf,0);
            buf += 16;
            k -= 16;
        } while (k >= 16);
        if (k != 0) do
        {
            s1 += *buf++;
            s2 += s1;
        } while (--k > 0);
        s1 %= LZO_BASE;
        s2 %= LZO_BASE;
    }
    return (s2 << 16) | s1;
}

#undef LZO_DO1
#undef LZO_DO2
#undef LZO_DO4
#undef LZO_DO8
#undef LZO_DO16


/*
vi:ts=4:et
*/

/* --- lzo1x_o.c */

/* lzo1x_o.c -- LZO1X compressed data optimizer

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


/* --- config1x.h */

/* config1x.h -- configuration for the LZO1X algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_CONFIG1X_H
#define __LZO_CONFIG1X_H

#if !defined(LZO1X) && !defined(LZO1Y) && !defined(LZO1Z)
#  define LZO1X
#endif

#include "prepcore_lzo.h"


/***********************************************************************
//
************************************************************************/

#define LZO_EOF_CODE
#undef LZO_DETERMINISTIC

#define M1_MAX_OFFSET   0x0400
#ifndef M2_MAX_OFFSET
#define M2_MAX_OFFSET   0x0800
#endif
#define M3_MAX_OFFSET   0x4000
#define M4_MAX_OFFSET   0xbfff

#define MX_MAX_OFFSET   (M1_MAX_OFFSET + M2_MAX_OFFSET)

#define M1_MIN_LEN      2
#define M1_MAX_LEN      2
#define M2_MIN_LEN      3
#ifndef M2_MAX_LEN
#define M2_MAX_LEN      8
#endif
#define M3_MIN_LEN      3
#define M3_MAX_LEN      33
#define M4_MIN_LEN      3
#define M4_MAX_LEN      9

#define M1_MARKER       0
#define M2_MARKER       64
#define M3_MARKER       32
#define M4_MARKER       16


/***********************************************************************
//
************************************************************************/

#ifndef MIN_LOOKAHEAD
#define MIN_LOOKAHEAD       (M2_MAX_LEN + 1)
#endif

#if defined(LZO_NEED_DICT_H)

#ifndef LZO_HASH
#define LZO_HASH            LZO_HASH_LZO_INCREMENTAL_B
#endif
#define DL_MIN_LEN          M2_MIN_LEN
#include "lzo_dict.h"

#endif



#endif /* already included */

/*
vi:ts=4:et
*/



/* --- */

#define DO_OPTIMIZE     lzo1x_optimize

/* --- lzo1x_oo.ch */

/* lzo1x_oo.ch -- LZO1X compressed data optimizer

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


#define TEST_IP     (ip < ip_end)
#define TEST_OP     (op <= op_end)

#define NO_LIT      LZO_UINT_MAX


/***********************************************************************
//
************************************************************************/

static void copy2(lzo_bytep ip, const lzo_bytep m_pos, lzo_uint off)
{
    assert(off > 0);
    ip[0] = m_pos[0];
    if (off == 1)
        ip[1] = m_pos[0];
    else
        ip[1] = m_pos[1];
}


static void copy3(lzo_bytep ip, const lzo_bytep m_pos, lzo_uint off)
{
    assert(off > 0);
    ip[0] = m_pos[0];
    if (off == 1)
    {
        ip[2] = ip[1] = m_pos[0];
    }
    else if (off == 2)
    {
        ip[1] = m_pos[1];
        ip[2] = m_pos[0];
    }
    else
    {
        ip[1] = m_pos[1];
        ip[2] = m_pos[2];
    }
}


/***********************************************************************
// optimize a block of data.
************************************************************************/

LZO_PUBLIC(int)
DO_OPTIMIZE          (       lzo_bytep in , lzo_uint  in_len,
                             lzo_bytep out, lzo_uintp out_len,
                             lzo_voidp wrkmem )
{
    lzo_bytep op;
    lzo_bytep ip;
    lzo_uint t;
    lzo_bytep m_pos;
    lzo_bytep const ip_end = in + in_len;
    lzo_bytep const op_end = out + *out_len;
    lzo_bytep litp = NULL;
    lzo_uint lit = 0;
    lzo_uint next_lit = NO_LIT;
    lzo_uint nl;
    unsigned long o_m1_a = 0, o_m1_b = 0, o_m2 = 0, o_m3_a = 0, o_m3_b = 0;

    LZO_UNUSED(wrkmem);

    *out_len = 0;

    op = out;
    ip = in;

    assert(in_len >= 3);
    if (*ip > 17)
    {
        t = *ip++ - 17;
        if (t < 4)
            goto match_next;
        goto first_literal_run;
    }
    assert(*ip < 16 || (*ip == 17 && in_len == 3));

    while (TEST_IP && TEST_OP)
    {
        t = *ip++;
        if (t >= 16)
            goto match;
        /* a literal run */
        litp = ip - 1;
        if (t == 0)
        {
            t = 15;
            while (*ip == 0)
                t += 255, ip++;
            t += *ip++;
        }
        lit = t + 3;
        /* copy literals */
copy_literal_run:
        *op++ = *ip++; *op++ = *ip++; *op++ = *ip++;
first_literal_run:
        do *op++ = *ip++; while (--t > 0);


        t = *ip++;

        if (t >= 16)
            goto match;
#if defined(LZO1X)
        m_pos = op - 1 - 0x800;
#elif defined(LZO1Y)
        m_pos = op - 1 - 0x400;
#endif
        m_pos -= t >> 2;
        m_pos -= *ip++ << 2;
        *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos++;
        lit = 0;
        goto match_done;


        /* handle matches */
        do {
            if (t < 16)                     /* a M1 match */
            {
                m_pos = op - 1;
                m_pos -= t >> 2;
                m_pos -= *ip++ << 2;

                if (litp == NULL)
                    goto copy_m1;

                /* assert that there was a match just before */
                assert(lit >= 1 && lit <= 3);
                assert(litp == ip - 2 - lit - 2);
                assert((lzo_uint)(*litp & 3) == lit);
                nl = ip[-2] & 3;
                /* test if a match follows */
                if (nl == 0 && lit == 1 && ip[0] >= 16)
                {
                    next_lit = nl;
                    /* adjust length of previous short run */
                    lit += 2;
                    *litp = LZO_BYTE((*litp & ~3) | lit);
                    /* copy over the 2 literals that replace the match */
                    copy2(ip-2,m_pos,pd(op,m_pos));
                    o_m1_a++;
                }
                /* test if a literal run follows */
                else if (nl == 0 && ip[0] < 16 && ip[0] != 0 &&
                         (lit + 2 + ip[0] < 16))
                {
                    t = *ip++;
                    /* remove short run */
                    *litp &= ~3;
                    /* copy over the 2 literals that replace the match */
                    copy2(ip-3+1,m_pos,pd(op,m_pos));
                    /* move literals 1 byte ahead */
                    litp += 2;
                    if (lit > 0)
                        lzo_memmove(litp+1,litp,lit);
                    /* insert new length of long literal run */
                    lit += 2 + t + 3; assert(lit <= 18);
                    *litp = LZO_BYTE(lit - 3);

                    o_m1_b++;
                    *op++ = *m_pos++; *op++ = *m_pos++;
                    goto copy_literal_run;
                }
copy_m1:
                *op++ = *m_pos++; *op++ = *m_pos++;
            }
            else
            {
match:
                if (t >= 64)                /* a M2 match */
                {
                    m_pos = op - 1;
#if defined(LZO1X)
                    m_pos -= (t >> 2) & 7;
                    m_pos -= *ip++ << 3;
                    t = (t >> 5) - 1;
#elif defined(LZO1Y)
                    m_pos -= (t >> 2) & 3;
                    m_pos -= *ip++ << 2;
                    t = (t >> 4) - 3;
#endif
                    if (litp == NULL)
                        goto copy_m;

                    nl = ip[-2] & 3;
                    /* test if in beetween two long literal runs */
                    if (t == 1 && lit > 3 && nl == 0 &&
                        ip[0] < 16 && ip[0] != 0 && (lit + 3 + ip[0] < 16))
                    {
                        assert(*litp == lit - 3);
                        t = *ip++;
                        /* copy over the 3 literals that replace the match */
                        copy3(ip-1-2,m_pos,pd(op,m_pos));
                        /* set new length of previous literal run */
                        lit += 3 + t + 3; assert(lit <= 18);
                        *litp = LZO_BYTE(lit - 3);
                        o_m2++;
                        *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos++;
                        goto copy_literal_run;
                    }
                }
                else
                {
                    if (t >= 32)            /* a M3 match */
                    {
                        t &= 31;
                        if (t == 0)
                        {
                            t = 31;
                            while (*ip == 0)
                                t += 255, ip++;
                            t += *ip++;
                        }
                        m_pos = op - 1;
                        m_pos -= *ip++ >> 2;
                        m_pos -= *ip++ << 6;
                    }
                    else                    /* a M4 match */
                    {
                        m_pos = op;
                        m_pos -= (t & 8) << 11;
                        t &= 7;
                        if (t == 0)
                        {
                            t = 7;
                            while (*ip == 0)
                                t += 255, ip++;
                            t += *ip++;
                        }
                        m_pos -= *ip++ >> 2;
                        m_pos -= *ip++ << 6;
                        if (m_pos == op)
                            goto eof_found;
                        m_pos -= 0x4000;
                    }
                    if (litp == NULL)
                        goto copy_m;

                    nl = ip[-2] & 3;
                    /* test if in beetween two matches */
                    if (t == 1 && lit == 0 && nl == 0 && ip[0] >= 16)
                    {
                        assert(litp == ip - 3 - lit - 2);
                        assert((lzo_uint)(*litp & 3) == lit);
                        next_lit = nl;
                        /* make a previous short run */
                        lit += 3;
                        *litp = LZO_BYTE((*litp & ~3) | lit);
                        /* copy over the 3 literals that replace the match */
                        copy3(ip-3,m_pos,pd(op,m_pos));
                        o_m3_a++;
                    }
                    /* test if a literal run follows */
                    else if (t == 1 && lit <= 3 && nl == 0 &&
                             ip[0] < 16 && ip[0] != 0 && (lit + 3 + ip[0] < 16))
                    {
                        assert(litp == ip - 3 - lit - 2);
                        assert((lzo_uint)(*litp & 3) == lit);
                        t = *ip++;
                        /* remove short run */
                        *litp &= ~3;
                        /* copy over the 3 literals that replace the match */
                        copy3(ip-4+1,m_pos,pd(op,m_pos));
                        /* move literals 1 byte ahead */
                        litp += 2;
                        if (lit > 0)
                            lzo_memmove(litp+1,litp,lit);
                        /* insert new length of long literal run */
                        lit += 3 + t + 3; assert(lit <= 18);
                        *litp = LZO_BYTE(lit - 3);

                        o_m3_b++;
                        *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos++;
                        goto copy_literal_run;
                    }
                }
copy_m:
                *op++ = *m_pos++; *op++ = *m_pos++;
                do *op++ = *m_pos++; while (--t > 0);
            }

match_done:
            if (next_lit == NO_LIT)
            {
                t = ip[-2] & 3;
                lit = t;
                litp = ip - 2;
            }
            else
                t = next_lit;
            assert(t <= 3);
            next_lit = NO_LIT;
            if (t == 0)
                break;
            /* copy literals */
match_next:
            do *op++ = *ip++; while (--t > 0);
            t = *ip++;
        } while (TEST_IP && TEST_OP);
    }

    /* no EOF code was found */
    *out_len = pd(op, out);
    return LZO_E_EOF_NOT_FOUND;

eof_found:
    assert(t == 1);
#if 0
    printf("optimize: %5lu %5lu   %5lu   %5lu %5lu\n",
           o_m1_a, o_m1_b, o_m2, o_m3_a, o_m3_b);
#endif
    LZO_UNUSED(o_m1_a); LZO_UNUSED(o_m1_b); LZO_UNUSED(o_m2);
    LZO_UNUSED(o_m3_a); LZO_UNUSED(o_m3_b);
    *out_len = pd(op, out);
    return (ip == ip_end ? LZO_E_OK :
           (ip < ip_end  ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));
}


/*
vi:ts=4:et
*/



/* --- */

/* --- lzo1x_9x.c */

/* lzo1x_9x.c -- implementation of the LZO1X-999 compression algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


#if !defined(LZO1X) && !defined(LZO1Y) && !defined(LZO1Z)
#  define LZO1X
#endif

#if defined(LZO1X)
#else
#  error
#endif


/***********************************************************************
//
************************************************************************/

#define N           M4_MAX_OFFSET   /* size of ring buffer */
#define THRESHOLD       1           /* lower limit for match length */
#define F            2048           /* upper limit for match length */

#define SWD_BEST_OFF    (LZO_MAX3( M2_MAX_LEN, M3_MAX_LEN, M4_MAX_LEN ) + 1)

#if defined(LZO1X)
#  define LZO_COMPRESS_T                lzo1x_999_t
#  define lzo_swd_t                     lzo1x_999_swd_t
#elif defined(LZO1Y)
#  define LZO_COMPRESS_T                lzo1y_999_t
#  define lzo_swd_t                     lzo1y_999_swd_t
#  define lzo1x_999_compress_internal   lzo1y_999_compress_internal
#  define lzo1x_999_compress_dict       lzo1y_999_compress_dict
#  define lzo1x_999_compress_level      lzo1y_999_compress_level
#  define lzo1x_999_compress            lzo1y_999_compress
#elif defined(LZO1Z)
#  define LZO_COMPRESS_T                lzo1z_999_t
#  define lzo_swd_t                     lzo1z_999_swd_t
#  define lzo1x_999_compress_internal   lzo1z_999_compress_internal
#  define lzo1x_999_compress_dict       lzo1z_999_compress_dict
#  define lzo1x_999_compress_level      lzo1z_999_compress_level
#  define lzo1x_999_compress            lzo1z_999_compress
#else
#  error
#endif

#if 0
#  define HEAD3(b,p) \
    ((((((lzo_xint)b[p]<<3)^b[p+1])<<3)^b[p+2]) & (SWD_HSIZE-1))
#endif
#if 0 && defined(LZO_UNALIGNED_OK_4) && defined(LZO_ABI_LITTLE_ENDIAN)
#  define HEAD3(b,p) \
    (((* (lzo_uint32p) &b[p]) ^ ((* (lzo_uint32p) &b[p])>>10)) & (SWD_HSIZE-1))
#endif

/* --- lzo_mchw.ch */ 

/* lzo_mchw.ch -- matching functions using a window

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


/***********************************************************************
//
************************************************************************/

typedef struct
{
    int init;

    lzo_uint look;          /* bytes in lookahead buffer */

    lzo_uint m_len;
    lzo_uint m_off;

    lzo_uint last_m_len;
    lzo_uint last_m_off;

    const lzo_bytep bp;
    const lzo_bytep ip;
    const lzo_bytep in;
    const lzo_bytep in_end;
    lzo_bytep out;

    lzo_callback_p cb;

    lzo_uint textsize;      /* text size counter */
    lzo_uint codesize;      /* code size counter */
    lzo_uint printcount;    /* counter for reporting progress every 1K bytes */

    /* some stats */
    unsigned long lit_bytes;
    unsigned long match_bytes;
    unsigned long rep_bytes;
    unsigned long lazy;

#if defined(LZO1B)
    lzo_uint r1_m_len;

    /* some stats */
    unsigned long r1_r, m3_r, m2_m, m3_m;
#endif

#if defined(LZO1C)
    lzo_uint r1_m_len;
    lzo_bytep m3;

    /* some stats */
    unsigned long r1_r, m3_r, m2_m, m3_m;
#endif

#if defined(LZO1F)
    lzo_uint r1_lit;
    lzo_uint r1_m_len;

    /* some stats */
    unsigned long r1_r, m2_m, m3_m;
#endif

#if defined(LZO1X) || defined(LZO1Y) || defined(LZO1Z)
    lzo_uint r1_lit;
    lzo_uint r1_m_len;

    /* some stats */
    unsigned long m1a_m, m1b_m, m2_m, m3_m, m4_m;
    unsigned long lit1_r, lit2_r, lit3_r;
#endif

#if defined(LZO2A)
    /* some stats */
    unsigned long m1, m2, m3, m4;
#endif
}
LZO_COMPRESS_T;


#if (LZO_CC_BORLANDC && LZO_ARCH_I086) && (__BORLANDC__ < 0x0450)
   /* work around a Borland C 3.1 bug */
#  define getbyte(c)  ((c).ip < (c).in_end ? (c).ip +=1, (c).ip[-1] : (-1))
#elif defined(__TURBOC__) && defined(__TOS__)
   /* work around a bug in Turbo C / Pure C (Atari ST) */
#  define getbyte(c)  ((c).ip < (c).in_end ? (int)(unsigned) *((c).ip)++ : (-1))
#else
#  define getbyte(c)  ((c).ip < (c).in_end ? *((c).ip)++ : (-1))
#endif

/* --- lzo_swd.ch */

/* lzo_swd.ch -- sliding window dictionary

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


#if (LZO_UINT_MAX < LZO_0xffffffffL)
#  error "LZO_UINT_MAX"
#endif


/***********************************************************************
//
************************************************************************/

#ifndef SWD_N
#  define SWD_N             N
#endif
#ifndef SWD_F
#  define SWD_F             F
#endif
#ifndef SWD_THRESHOLD
#  define SWD_THRESHOLD     THRESHOLD
#endif

/* unsigned type for dictionary access - don't waste memory here */
#if (0UL + SWD_N + SWD_F + SWD_F < 0UL + USHRT_MAX)
   typedef unsigned short   swd_uint;
#  define SWD_UINT_MAX      USHRT_MAX
#elif (0UL + SWD_N + SWD_F + SWD_F < 0UL + UINT_MAX)
   typedef unsigned         swd_uint;
#  define SWD_UINT_MAX      UINT_MAX
#else
   typedef lzo_uint         swd_uint;
#  define SWD_UINT_MAX      LZO_UINT_MAX
#endif
#define swd_uintp           swd_uint __LZO_MMODEL *
#define SWD_UINT(x)         ((swd_uint)(x))


#ifndef SWD_HSIZE
#  define SWD_HSIZE         16384
#endif
#ifndef SWD_MAX_CHAIN
#  define SWD_MAX_CHAIN     2048
#endif

#if !defined(HEAD3)
#if 1
#  define HEAD3(b,p) \
    ((DMUL(0x9f5f,(((((lzo_xint)b[p]<<5)^b[p+1])<<5)^b[p+2]))>>5) & (SWD_HSIZE-1))
#else
#  define HEAD3(b,p) \
    ((DMUL(0x9f5f,(((((lzo_xint)b[p+2]<<5)^b[p+1])<<5)^b[p]))>>5) & (SWD_HSIZE-1))
#endif
#endif

#if (SWD_THRESHOLD == 1) && !defined(HEAD2)
#  if 1 && defined(LZO_UNALIGNED_OK_2)
#    define HEAD2(b,p)      (* (lzo_ushortp) &(b[p]))
#  else
#    define HEAD2(b,p)      (b[p] ^ ((unsigned)b[p+1]<<8))
#  endif
#  define NIL2              SWD_UINT_MAX
#endif


typedef struct
{
/* public - "built-in" */
    lzo_uint n;
    lzo_uint f;
    lzo_uint threshold;

/* public - configuration */
    lzo_uint max_chain;
    lzo_uint nice_length;
    lzo_bool use_best_off;
    lzo_uint lazy_insert;

/* public - output */
    lzo_uint m_len;
    lzo_uint m_off;
    lzo_uint look;
    int b_char;
#if defined(SWD_BEST_OFF)
    lzo_uint best_off[ SWD_BEST_OFF ];
#endif

/* semi public */
    LZO_COMPRESS_T *c;
    lzo_uint m_pos;
#if defined(SWD_BEST_OFF)
    lzo_uint best_pos[ SWD_BEST_OFF ];
#endif

/* private */
    const lzo_bytep dict;
    const lzo_bytep dict_end;
    lzo_uint dict_len;

/* private */
    lzo_uint ip;                /* input pointer (lookahead) */
    lzo_uint bp;                /* buffer pointer */
    lzo_uint rp;                /* remove pointer */
    lzo_uint b_size;

    lzo_bytep b_wrap;

    lzo_uint node_count;
    lzo_uint first_rp;

#if defined(__LZO_MMODEL_HUGE)
#  define A(type, n)    ((((n) * sizeof(type)) + 3UL) &~ 3UL)

#  define O_b(s)        (0L)
#  define O_head3(s)    (O_b(s) + A(char, 0UL + SWD_N + SWD_F + SWD_F))
#  define O_succ3(s)    (O_head3(s) + A(swd_uint, 0UL + SWD_HSIZE))
#  define O_best3(s)    (O_succ3(s) + A(swd_uint, 0UL + SWD_N + SWD_F))
#  define O_llen3(s)    (O_best3(s) + A(swd_uint, 0UL + SWD_N + SWD_F))
# ifdef HEAD2
#  define O_head2(s)    (O_llen3(s) + A(swd_uint, 0UL + SWD_HSIZE))
#  define O_END(s)      (O_head2(s) + A(swd_uint, 0UL + 65536L))
# else
#  define O_END(s)      (O_llen3(s) + A(swd_uint, 0UL + SWD_HSIZE))
# endif

#  define S_DEF(s,type,off)  ((type) ((lzo_bytep)s + 0L + sizeof(*s) + off))
#  define s_b(s)        S_DEF(s, lzo_bytep, O_b(s))
#  define s_head3(s)    S_DEF(s, swd_uintp, O_head3(s))
#  define s_succ3(s)    S_DEF(s, swd_uintp, O_succ3(s))
#  define s_best3(s)    S_DEF(s, swd_uintp, O_best3(s))
#  define s_llen3(s)    S_DEF(s, swd_uintp, O_llen3(s))
# ifdef HEAD2
#  define s_head2(s)    S_DEF(s, swd_uintp, O_head2(s))
# endif

#elif defined(__LZO_CHECKER)
    /* malloc arrays of the exact size to detect any overrun */
    unsigned char *b;
    swd_uint *head3;
    swd_uint *succ3;
    swd_uint *best3;
    swd_uint *llen3;
# ifdef HEAD2
    swd_uint *head2;
# endif

#else
    unsigned char b [ SWD_N + SWD_F + SWD_F ];
    swd_uint head3 [ SWD_HSIZE ];
    swd_uint succ3 [ SWD_N + SWD_F ];
    swd_uint best3 [ SWD_N + SWD_F ];
    swd_uint llen3 [ SWD_HSIZE ];
# ifdef HEAD2
    swd_uint head2 [ 65536L ];
# endif
#endif
}
lzo_swd_t;
#define lzo_swd_p   lzo_swd_t __LZO_MMODEL *


#if defined(__LZO_MMODEL_HUGE)
#define SIZEOF_LZO_SWD_T    O_END(0)
#else
#define s_b(s)      s->b
#define s_head3(s)  s->head3
#define s_succ3(s)  s->succ3
#define s_best3(s)  s->best3
#define s_llen3(s)  s->llen3
#ifdef HEAD2
#define s_head2(s)  s->head2
#endif
#define SIZEOF_LZO_SWD_T    (sizeof(lzo_swd_t))
#endif


/* Access macro for head3.
 * head3[key] may be uninitialized, but then its value will never be used.
 */
#if defined(__LZO_CHECKER)
#  define s_get_head3(s,key) \
        ((s->llen3[key] == 0) ? SWD_UINT_MAX : s_head3(s)[key])
#else
#  define s_get_head3(s,key)    s_head3(s)[key]
#endif


/***********************************************************************
//
************************************************************************/

static
void swd_initdict(lzo_swd_p s, const lzo_bytep dict, lzo_uint dict_len)
{
    s->dict = s->dict_end = NULL;
    s->dict_len = 0;

    if (!dict || dict_len <= 0)
        return;
    if (dict_len > s->n)
    {
        dict += dict_len - s->n;
        dict_len = s->n;
    }

    s->dict = dict;
    s->dict_len = dict_len;
    s->dict_end = dict + dict_len;
    lzo_memcpy(s_b(s),dict,dict_len);
    s->ip = dict_len;
}


static
void swd_insertdict(lzo_swd_p s, lzo_uint node, lzo_uint len)
{
    lzo_uint key;

    s->node_count = s->n - len;
    s->first_rp = node;

    while (len-- > 0)
    {
        key = HEAD3(s_b(s),node);
        s_succ3(s)[node] = s_get_head3(s,key);
        s_head3(s)[key] = SWD_UINT(node);
        s_best3(s)[node] = SWD_UINT(s->f + 1);
        s_llen3(s)[key]++;
        assert(s_llen3(s)[key] <= SWD_N);

#ifdef HEAD2
        key = HEAD2(s_b(s),node);
        s_head2(s)[key] = SWD_UINT(node);
#endif

        node++;
    }
}


/***********************************************************************
//
************************************************************************/

static
int swd_init(lzo_swd_p s, const lzo_bytep dict, lzo_uint dict_len)
{
    lzo_uint i = 0;
    int c = 0;

#if defined(__LZO_CHECKER)
    s->b = malloc(SWD_N + SWD_F + SWD_F);
    s->head3 = malloc(sizeof(swd_uint) * SWD_HSIZE);
    s->succ3 = malloc(sizeof(swd_uint) * (SWD_N + SWD_F));
    s->best3 = malloc(sizeof(swd_uint) * (SWD_N + SWD_F));
    s->llen3 = malloc(sizeof(swd_uint) * SWD_HSIZE);
#ifdef HEAD2
    s->head2 = malloc(sizeof(swd_uint) * 65536L);
#endif
#endif

    s->n = SWD_N;
    s->f = SWD_F;
    s->threshold = SWD_THRESHOLD;

    /* defaults */
    s->max_chain = SWD_MAX_CHAIN;
    s->nice_length = SWD_F;
    s->use_best_off = 0;
    s->lazy_insert = 0;

    s->b_size = s->n + s->f;
#if 0
    if (2 * s->f >= s->n || s->b_size + s->f >= SWD_UINT_MAX)
        return LZO_E_ERROR;
#else
    LZO_COMPILE_TIME_ASSERT(!(0ul + 2 * SWD_F >= SWD_N))
    LZO_COMPILE_TIME_ASSERT(!(0ul + SWD_N + SWD_F + SWD_F >= SWD_UINT_MAX))
#endif
    s->b_wrap = s_b(s) + s->b_size;
    s->node_count = s->n;

    lzo_memset(s_llen3(s), 0, sizeof(s_llen3(s)[0]) * (lzo_uint)SWD_HSIZE);
#ifdef HEAD2
#if 1
    lzo_memset(s_head2(s), 0xff, sizeof(s_head2(s)[0]) * 65536L);
    assert(s_head2(s)[0] == NIL2);
#else
    for (i = 0; i < 65536L; i++)
        s_head2(s)[i] = NIL2;
#endif
#endif

    s->ip = 0;
    swd_initdict(s,dict,dict_len);
    s->bp = s->ip;
    s->first_rp = s->ip;

    assert(s->ip + s->f <= s->b_size);
#if 1
    s->look = (lzo_uint) (s->c->in_end - s->c->ip);
    if (s->look > 0)
    {
        if (s->look > s->f)
            s->look = s->f;
        lzo_memcpy(&s_b(s)[s->ip],s->c->ip,s->look);
        s->c->ip += s->look;
        s->ip += s->look;
    }
#else
    s->look = 0;
    while (s->look < s->f)
    {
        if ((c = getbyte(*(s->c))) < 0)
            break;
        s_b(s)[s->ip] = LZO_BYTE(c);
        s->ip++;
        s->look++;
    }
#endif
    if (s->ip == s->b_size)
        s->ip = 0;

    if (s->look >= 2 && s->dict_len > 0)
        swd_insertdict(s,0,s->dict_len);

    s->rp = s->first_rp;
    if (s->rp >= s->node_count)
        s->rp -= s->node_count;
    else
        s->rp += s->b_size - s->node_count;

#if defined(__LZO_CHECKER)
    /* initialize memory for the first few HEAD3 (if s->ip is not far
     * enough ahead to do this job for us). The value doesn't matter. */
    if (s->look < 3)
        lzo_memset(&s_b(s)[s->bp+s->look],0,3);
#endif

    LZO_UNUSED(i);
    LZO_UNUSED(c);
    return LZO_E_OK;
}


static
void swd_exit(lzo_swd_p s)
{
#if defined(__LZO_CHECKER)
    /* free in reverse order of allocations */
#ifdef HEAD2
    free(s->head2); s->head2 = NULL;
#endif
    free(s->llen3); s->llen3 = NULL;
    free(s->best3); s->best3 = NULL;
    free(s->succ3); s->succ3 = NULL;
    free(s->head3); s->head3 = NULL;
    free(s->b); s->b = NULL;
#else
    LZO_UNUSED(s);
#endif
}


#define swd_pos2off(s,pos) \
    (s->bp > (pos) ? s->bp - (pos) : s->b_size - ((pos) - s->bp))


/***********************************************************************
//
************************************************************************/

static __lzo_inline
void swd_getbyte(lzo_swd_p s)
{
    int c;

    if ((c = getbyte(*(s->c))) < 0)
    {
        if (s->look > 0)
            --s->look;
#if defined(__LZO_CHECKER)
        /* initialize memory - value doesn't matter */
        s_b(s)[s->ip] = 0;
        if (s->ip < s->f)
            s->b_wrap[s->ip] = 0;
#endif
    }
    else
    {
        s_b(s)[s->ip] = LZO_BYTE(c);
        if (s->ip < s->f)
            s->b_wrap[s->ip] = LZO_BYTE(c);
    }
    if (++s->ip == s->b_size)
        s->ip = 0;
    if (++s->bp == s->b_size)
        s->bp = 0;
    if (++s->rp == s->b_size)
        s->rp = 0;
}


/***********************************************************************
// remove node from lists
************************************************************************/

static __lzo_inline
void swd_remove_node(lzo_swd_p s, lzo_uint node)
{
    if (s->node_count == 0)
    {
        lzo_uint key;

#ifdef LZO_DEBUG
        if (s->first_rp != LZO_UINT_MAX)
        {
            if (node != s->first_rp)
                printf("Remove %5u: %5u %5u %5u %5u  %6u %6u\n",
                        node, s->rp, s->ip, s->bp, s->first_rp,
                        s->ip - node, s->ip - s->bp);
            assert(node == s->first_rp);
            s->first_rp = LZO_UINT_MAX;
        }
#endif

        key = HEAD3(s_b(s),node);
        assert(s_llen3(s)[key] > 0);
        --s_llen3(s)[key];

#ifdef HEAD2
        key = HEAD2(s_b(s),node);
        assert(s_head2(s)[key] != NIL2);
        if ((lzo_uint) s_head2(s)[key] == node)
            s_head2(s)[key] = NIL2;
#endif
    }
    else
        --s->node_count;
}


/***********************************************************************
//
************************************************************************/

static
void swd_accept(lzo_swd_p s, lzo_uint n)
{
    assert(n <= s->look);

    while (n--)
    {
        lzo_uint key;

        swd_remove_node(s,s->rp);

        /* add bp into HEAD3 */
        key = HEAD3(s_b(s),s->bp);
        s_succ3(s)[s->bp] = s_get_head3(s,key);
        s_head3(s)[key] = SWD_UINT(s->bp);
        s_best3(s)[s->bp] = SWD_UINT(s->f + 1);
        s_llen3(s)[key]++;
        assert(s_llen3(s)[key] <= SWD_N);

#ifdef HEAD2
        /* add bp into HEAD2 */
        key = HEAD2(s_b(s),s->bp);
        s_head2(s)[key] = SWD_UINT(s->bp);
#endif

        swd_getbyte(s);
    }
}


/***********************************************************************
//
************************************************************************/

static
void swd_search(lzo_swd_p s, lzo_uint node, lzo_uint cnt)
{
    const lzo_bytep p1;
    const lzo_bytep p2;
    const lzo_bytep px;
    lzo_uint m_len = s->m_len;
    const lzo_bytep b  = s_b(s);
    const lzo_bytep bp = s_b(s) + s->bp;
    const lzo_bytep bx = s_b(s) + s->bp + s->look;
    unsigned char scan_end1;

    assert(s->m_len > 0);

    scan_end1 = bp[m_len - 1];
    for ( ; cnt-- > 0; node = s_succ3(s)[node])
    {
        p1 = bp;
        p2 = b + node;
        px = bx;

        assert(m_len < s->look);

        if (
#if 1
            p2[m_len - 1] == scan_end1 &&
            p2[m_len] == p1[m_len] &&
#endif
            p2[0] == p1[0] &&
            p2[1] == p1[1])
        {
            lzo_uint i;
            assert(lzo_memcmp(bp,&b[node],3) == 0);

#if 0 && defined(LZO_UNALIGNED_OK_4)
            p1 += 3; p2 += 3;
            while (p1 < px && * (const lzo_uint32p) p1 == * (const lzo_uint32p) p2)
                p1 += 4, p2 += 4;
            while (p1 < px && *p1 == *p2)
                p1 += 1, p2 += 1;
#else
            p1 += 2; p2 += 2;
            do {} while (++p1 < px && *p1 == *++p2);
#endif
            i = pd(p1, bp);

#ifdef LZO_DEBUG
            if (lzo_memcmp(bp,&b[node],i) != 0)
                printf("%5ld %5ld %02x%02x %02x%02x\n",
                        (long)s->bp, (long) node,
                        bp[0], bp[1], b[node], b[node+1]);
#endif
            assert(lzo_memcmp(bp,&b[node],i) == 0);

#if defined(SWD_BEST_OFF)
            if (i < SWD_BEST_OFF)
            {
                if (s->best_pos[i] == 0)
                    s->best_pos[i] = node + 1;
            }
#endif
            if (i > m_len)
            {
                s->m_len = m_len = i;
                s->m_pos = node;
                if (m_len == s->look)
                    return;
                if (m_len >= s->nice_length)
                    return;
                if (m_len > (lzo_uint) s_best3(s)[node])
                    return;
                scan_end1 = bp[m_len - 1];
            }
        }
    }
}


/***********************************************************************
//
************************************************************************/

#ifdef HEAD2

static
lzo_bool swd_search2(lzo_swd_p s)
{
    lzo_uint key;

    assert(s->look >= 2);
    assert(s->m_len > 0);

    key = s_head2(s)[ HEAD2(s_b(s),s->bp) ];
    if (key == NIL2)
        return 0;
#ifdef LZO_DEBUG
    if (lzo_memcmp(&s_b(s)[s->bp],&s_b(s)[key],2) != 0)
        printf("%5ld %5ld %02x%02x %02x%02x\n", (long)s->bp, (long)key,
                s_b(s)[s->bp], s_b(s)[s->bp+1], s_b(s)[key], s_b(s)[key+1]);
#endif
    assert(lzo_memcmp(&s_b(s)[s->bp],&s_b(s)[key],2) == 0);
#if defined(SWD_BEST_OFF)
    if (s->best_pos[2] == 0)
        s->best_pos[2] = key + 1;
#endif

    if (s->m_len < 2)
    {
        s->m_len = 2;
        s->m_pos = key;
    }
    return 1;
}

#endif


/***********************************************************************
//
************************************************************************/

static
void swd_findbest(lzo_swd_p s)
{
    lzo_uint key;
    lzo_uint cnt, node;
    lzo_uint len;

    assert(s->m_len > 0);

    /* get current head, add bp into HEAD3 */
    key = HEAD3(s_b(s),s->bp);
    node = s_succ3(s)[s->bp] = s_get_head3(s,key);
    cnt = s_llen3(s)[key]++;
    assert(s_llen3(s)[key] <= SWD_N + SWD_F);
    if (cnt > s->max_chain && s->max_chain > 0)
        cnt = s->max_chain;
    s_head3(s)[key] = SWD_UINT(s->bp);

    s->b_char = s_b(s)[s->bp];
    len = s->m_len;
    if (s->m_len >= s->look)
    {
        if (s->look == 0)
            s->b_char = -1;
        s->m_off = 0;
        s_best3(s)[s->bp] = SWD_UINT(s->f + 1);
    }
    else
    {
#ifdef HEAD2
        if (swd_search2(s))
#endif
            if (s->look >= 3)
                swd_search(s,node,cnt);
        if (s->m_len > len)
            s->m_off = swd_pos2off(s,s->m_pos);
        s_best3(s)[s->bp] = SWD_UINT(s->m_len);

#if defined(SWD_BEST_OFF)
        if (s->use_best_off)
        {
            int i;
            for (i = 2; i < SWD_BEST_OFF; i++)
                if (s->best_pos[i] > 0)
                    s->best_off[i] = swd_pos2off(s,s->best_pos[i]-1);
                else
                    s->best_off[i] = 0;
        }
#endif
    }

    swd_remove_node(s,s->rp);

#ifdef HEAD2
    /* add bp into HEAD2 */
    key = HEAD2(s_b(s),s->bp);
    s_head2(s)[key] = SWD_UINT(s->bp);
#endif
}


#undef HEAD3
#undef HEAD2
#undef s_get_head3


/*
vi:ts=4:et
*/


/* --- */


/***********************************************************************
//
************************************************************************/

static int
init_match ( LZO_COMPRESS_T *c, lzo_swd_p s,
             const lzo_bytep dict, lzo_uint dict_len,
             lzo_uint32 flags )
{
    int r;

    assert(!c->init);
    c->init = 1;

    s->c = c;

    c->last_m_len = c->last_m_off = 0;

    c->textsize = c->codesize = c->printcount = 0;
    c->lit_bytes = c->match_bytes = c->rep_bytes = 0;
    c->lazy = 0;

    r = swd_init(s,dict,dict_len);
    if (r != 0)
        return r;

    s->use_best_off = (flags & 1) ? 1 : 0;
    return r;
}


/***********************************************************************
//
************************************************************************/

static int
find_match ( LZO_COMPRESS_T *c, lzo_swd_p s,
             lzo_uint this_len, lzo_uint skip )
{
    assert(c->init);

    if (skip > 0)
    {
        assert(this_len >= skip);
        swd_accept(s, this_len - skip);
        c->textsize += this_len - skip + 1;
    }
    else
    {
        assert(this_len <= 1);
        c->textsize += this_len - skip;
    }

    s->m_len = 1;
    s->m_len = THRESHOLD;
#ifdef SWD_BEST_OFF
    if (s->use_best_off)
        lzo_memset(s->best_pos,0,sizeof(s->best_pos));
#endif
    swd_findbest(s);
    c->m_len = s->m_len;
    c->m_off = s->m_off;

    swd_getbyte(s);

    if (s->b_char < 0)
    {
        c->look = 0;
        c->m_len = 0;
        swd_exit(s);
    }
    else
    {
        c->look = s->look + 1;
    }
    c->bp = c->ip - c->look;

#if 0
    /* brute force match search */
    if (c->m_len > THRESHOLD && c->m_len + 1 <= c->look)
    {
        const lzo_bytep ip = c->bp;
        const lzo_bytep m  = c->bp - c->m_off;
        const lzo_bytep in = c->in;

        if (ip - in > N)
            in = ip - N;
        for (;;)
        {
            while (*in != *ip)
                in++;
            if (in == ip)
                break;
            if (in != m)
                if (lzo_memcmp(in,ip,c->m_len+1) == 0)
                    printf("%p %p %p %5d\n",in,ip,m,c->m_len);
            in++;
        }
    }
#endif

    if (c->cb && c->cb->nprogress && c->textsize > c->printcount)
    {
        (*c->cb->nprogress)(c->cb, c->textsize, c->codesize, 0);
        c->printcount += 1024;
    }

    return LZO_E_OK;
}


/*
vi:ts=4:et
*/


/* --- */


/* this is a public functions, but there is no prototype in a header file */
LZO_EXTERN(int)
lzo1x_999_compress_internal ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len,
                                    lzo_callback_p cb,
                                    int try_lazy,
                                    lzo_uint good_length,
                                    lzo_uint max_lazy,
                                    lzo_uint nice_length,
                                    lzo_uint max_chain,
                                    lzo_uint32 flags );


/***********************************************************************
//
************************************************************************/

static lzo_bytep
code_match ( LZO_COMPRESS_T *c, lzo_bytep op, lzo_uint m_len, lzo_uint m_off )
{
    lzo_uint x_len = m_len;
    lzo_uint x_off = m_off;

    c->match_bytes += (unsigned long) m_len;

#if 0
/*
    static lzo_uint last_m_len = 0, last_m_off = 0;
    static lzo_uint prev_m_off[4];
    static int prev_m_off_ptr = 0;
    int i;

    //if (m_len >= 3 && m_len <= M2_MAX_LEN && m_off <= M2_MAX_OFFSET)
    if (m_len >= 3 && m_len <= M2_MAX_LEN)
    {
    //if (m_len == last_m_len && m_off == last_m_off)
        //printf("last_m_len + last_m_off\n");
    //else
    if (m_off == last_m_off)
        printf("last_m_off\n");
    else
    {
        for (i = 0; i < 4; i++)
            if (m_off == prev_m_off[i])
                printf("prev_m_off %d: %5ld\n",i,(long)m_off);
    }
    }
    last_m_len = m_len;
    last_m_off = prev_m_off[prev_m_off_ptr] = m_off;
    prev_m_off_ptr = (prev_m_off_ptr + 1) & 3;
*/
#endif

    assert(op > c->out);
    if (m_len == 2)
    {
        assert(m_off <= M1_MAX_OFFSET);
        assert(c->r1_lit > 0); assert(c->r1_lit < 4);
        m_off -= 1;
#if defined(LZO1Z)
        *op++ = LZO_BYTE(M1_MARKER | (m_off >> 6));
        *op++ = LZO_BYTE(m_off << 2);
#else
        *op++ = LZO_BYTE(M1_MARKER | ((m_off & 3) << 2));
        *op++ = LZO_BYTE(m_off >> 2);
#endif
        c->m1a_m++;
    }
#if defined(LZO1Z)
    else if (m_len <= M2_MAX_LEN && (m_off <= M2_MAX_OFFSET || m_off == c->last_m_off))
#else
    else if (m_len <= M2_MAX_LEN && m_off <= M2_MAX_OFFSET)
#endif
    {
        assert(m_len >= 3);
#if defined(LZO1X)
        m_off -= 1;
        *op++ = LZO_BYTE(((m_len - 1) << 5) | ((m_off & 7) << 2));
        *op++ = LZO_BYTE(m_off >> 3);
        assert(op[-2] >= M2_MARKER);
#elif defined(LZO1Y)
        m_off -= 1;
        *op++ = LZO_BYTE(((m_len + 1) << 4) | ((m_off & 3) << 2));
        *op++ = LZO_BYTE(m_off >> 2);
        assert(op[-2] >= M2_MARKER);
#elif defined(LZO1Z)
        if (m_off == c->last_m_off)
            *op++ = LZO_BYTE(((m_len - 1) << 5) | (0x700 >> 6));
        else
        {
            m_off -= 1;
            *op++ = LZO_BYTE(((m_len - 1) << 5) | (m_off >> 6));
            *op++ = LZO_BYTE(m_off << 2);
        }
#endif
        c->m2_m++;
    }
    else if (m_len == M2_MIN_LEN && m_off <= MX_MAX_OFFSET && c->r1_lit >= 4)
    {
        assert(m_len == 3);
        assert(m_off > M2_MAX_OFFSET);
        m_off -= 1 + M2_MAX_OFFSET;
#if defined(LZO1Z)
        *op++ = LZO_BYTE(M1_MARKER | (m_off >> 6));
        *op++ = LZO_BYTE(m_off << 2);
#else
        *op++ = LZO_BYTE(M1_MARKER | ((m_off & 3) << 2));
        *op++ = LZO_BYTE(m_off >> 2);
#endif
        c->m1b_m++;
    }
    else if (m_off <= M3_MAX_OFFSET)
    {
        assert(m_len >= 3);
        m_off -= 1;
        if (m_len <= M3_MAX_LEN)
            *op++ = LZO_BYTE(M3_MARKER | (m_len - 2));
        else
        {
            m_len -= M3_MAX_LEN;
            *op++ = M3_MARKER | 0;
            while (m_len > 255)
            {
                m_len -= 255;
                *op++ = 0;
            }
            assert(m_len > 0);
            *op++ = LZO_BYTE(m_len);
        }
#if defined(LZO1Z)
        *op++ = LZO_BYTE(m_off >> 6);
        *op++ = LZO_BYTE(m_off << 2);
#else
        *op++ = LZO_BYTE(m_off << 2);
        *op++ = LZO_BYTE(m_off >> 6);
#endif
        c->m3_m++;
    }
    else
    {
        lzo_uint k;

        assert(m_len >= 3);
        assert(m_off > 0x4000); assert(m_off <= 0xbfff);
        m_off -= 0x4000;
        k = (m_off & 0x4000) >> 11;
        if (m_len <= M4_MAX_LEN)
            *op++ = LZO_BYTE(M4_MARKER | k | (m_len - 2));
        else
        {
            m_len -= M4_MAX_LEN;
            *op++ = LZO_BYTE(M4_MARKER | k | 0);
            while (m_len > 255)
            {
                m_len -= 255;
                *op++ = 0;
            }
            assert(m_len > 0);
            *op++ = LZO_BYTE(m_len);
        }
#if defined(LZO1Z)
        *op++ = LZO_BYTE(m_off >> 6);
        *op++ = LZO_BYTE(m_off << 2);
#else
        *op++ = LZO_BYTE(m_off << 2);
        *op++ = LZO_BYTE(m_off >> 6);
#endif
        c->m4_m++;
    }

    c->last_m_len = x_len;
    c->last_m_off = x_off;
    return op;
}


static lzo_bytep
STORE_RUN ( LZO_COMPRESS_T *c, lzo_bytep op, const lzo_bytep ii, lzo_uint t )
{
    c->lit_bytes += (unsigned long) t;

    if (op == c->out && t <= 238)
    {
        *op++ = LZO_BYTE(17 + t);
    }
    else if (t <= 3)
    {
#if defined(LZO1Z)
        op[-1] |= LZO_BYTE(t);
#else
        op[-2] |= LZO_BYTE(t);
#endif
        c->lit1_r++;
    }
    else if (t <= 18)
    {
        *op++ = LZO_BYTE(t - 3);
        c->lit2_r++;
    }
    else
    {
        lzo_uint tt = t - 18;

        *op++ = 0;
        while (tt > 255)
        {
            tt -= 255;
            *op++ = 0;
        }
        assert(tt > 0);
        *op++ = LZO_BYTE(tt);
        c->lit3_r++;
    }
    do *op++ = *ii++; while (--t > 0);

    return op;
}


static lzo_bytep
code_run ( LZO_COMPRESS_T *c, lzo_bytep op, const lzo_bytep ii,
           lzo_uint lit, lzo_uint m_len )
{
    if (lit > 0)
    {
        assert(m_len >= 2);
        op = STORE_RUN(c,op,ii,lit);
        c->r1_m_len = m_len;
        c->r1_lit = lit;
    }
    else
    {
        assert(m_len >= 3);
        c->r1_m_len = 0;
        c->r1_lit = 0;
    }

    return op;
}


/***********************************************************************
//
************************************************************************/

static int
len_of_coded_match ( lzo_uint m_len, lzo_uint m_off, lzo_uint lit )
{
    int n = 4;

    if (m_len < 2)
        return -1;
    if (m_len == 2)
        return (m_off <= M1_MAX_OFFSET && lit > 0 && lit < 4) ? 2 : -1;
    if (m_len <= M2_MAX_LEN && m_off <= M2_MAX_OFFSET)
        return 2;
    if (m_len == M2_MIN_LEN && m_off <= MX_MAX_OFFSET && lit >= 4)
        return 2;
    if (m_off <= M3_MAX_OFFSET)
    {
        if (m_len <= M3_MAX_LEN)
            return 3;
        m_len -= M3_MAX_LEN;
        while (m_len > 255)
        {
            m_len -= 255;
            n++;
        }
        return n;
    }
    if (m_off <= M4_MAX_OFFSET)
    {
        if (m_len <= M4_MAX_LEN)
            return 3;
        m_len -= M4_MAX_LEN;
        while (m_len > 255)
        {
            m_len -= 255;
            n++;
        }
        return n;
    }
    return -1;
}


static lzo_int
min_gain(lzo_uint ahead, lzo_uint lit1, lzo_uint lit2, int l1, int l2, int l3)
{
    lzo_int lazy_match_min_gain = 0;

    assert (ahead >= 1);
    lazy_match_min_gain += ahead;

#if 0
    if (l3 > 0)
        lit2 -= ahead;
#endif

    if (lit1 <= 3)
        lazy_match_min_gain += (lit2 <= 3) ? 0 : 2;
    else if (lit1 <= 18)
        lazy_match_min_gain += (lit2 <= 18) ? 0 : 1;

    lazy_match_min_gain += (l2 - l1) * 2;
    if (l3 > 0)
        lazy_match_min_gain -= (ahead - l3) * 2;

    if (lazy_match_min_gain < 0)
        lazy_match_min_gain = 0;

#if 0
    if (l1 == 2)
        if (lazy_match_min_gain == 0)
            lazy_match_min_gain = 1;
#endif

    return lazy_match_min_gain;
}


/***********************************************************************
//
************************************************************************/

#if !defined(NDEBUG)
static
void assert_match( const lzo_swd_p swd, lzo_uint m_len, lzo_uint m_off )
{
    const LZO_COMPRESS_T *c = swd->c;
    lzo_uint d_off;

    assert(m_len >= 2);
    if (m_off <= (lzo_uint) (c->bp - c->in))
    {
        assert(c->bp - m_off + m_len < c->ip);
        assert(lzo_memcmp(c->bp, c->bp - m_off, m_len) == 0);
    }
    else
    {
        assert(swd->dict != NULL);
        d_off = m_off - (lzo_uint) (c->bp - c->in);
        assert(d_off <= swd->dict_len);
        if (m_len > d_off)
        {
            assert(lzo_memcmp(c->bp, swd->dict_end - d_off, d_off) == 0);
            assert(c->in + m_len - d_off < c->ip);
            assert(lzo_memcmp(c->bp + d_off, c->in, m_len - d_off) == 0);
        }
        else
        {
            assert(lzo_memcmp(c->bp, swd->dict_end - d_off, m_len) == 0);
        }
    }
}
#else
#  define assert_match(a,b,c)   ((void)0)
#endif


#if defined(SWD_BEST_OFF)

static void
better_match ( const lzo_swd_p swd, lzo_uint *m_len, lzo_uint *m_off )
{
#if defined(LZO1Z)
    const LZO_COMPRESS_T *c = swd->c;
#endif

    if (*m_len <= M2_MIN_LEN)
        return;
#if defined(LZO1Z)
    if (*m_off == c->last_m_off && *m_len <= M2_MAX_LEN)
        return;
#if 1
    if (*m_len >= M2_MIN_LEN + 1 && *m_len <= M2_MAX_LEN + 1 &&
        c->last_m_off && swd->best_off[*m_len-1] == c->last_m_off)
    {
        *m_len = *m_len - 1;
        *m_off = swd->best_off[*m_len];
        return;
    }
#endif
#endif

    if (*m_off <= M2_MAX_OFFSET)
        return;

#if 1
    /* M3/M4 -> M2 */
    if (*m_off > M2_MAX_OFFSET &&
        *m_len >= M2_MIN_LEN + 1 && *m_len <= M2_MAX_LEN + 1 &&
        swd->best_off[*m_len-1] && swd->best_off[*m_len-1] <= M2_MAX_OFFSET)
    {
        *m_len = *m_len - 1;
        *m_off = swd->best_off[*m_len];
        return;
    }
#endif

#if 1
    /* M4 -> M2 */
    if (*m_off > M3_MAX_OFFSET &&
        *m_len >= M4_MAX_LEN + 1 && *m_len <= M2_MAX_LEN + 2 &&
        swd->best_off[*m_len-2] && swd->best_off[*m_len-2] <= M2_MAX_OFFSET)
    {
        *m_len = *m_len - 2;
        *m_off = swd->best_off[*m_len];
        return;
    }
#endif

#if 1
    /* M4 -> M3 */
    if (*m_off > M3_MAX_OFFSET &&
        *m_len >= M4_MAX_LEN + 1 && *m_len <= M3_MAX_LEN + 1 &&
        swd->best_off[*m_len-1] && swd->best_off[*m_len-1] <= M3_MAX_OFFSET)
    {
        *m_len = *m_len - 1;
        *m_off = swd->best_off[*m_len];
    }
#endif
}

#endif


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(int)
lzo1x_999_compress_internal ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len,
                                    lzo_callback_p cb,
                                    int try_lazy,
                                    lzo_uint good_length,
                                    lzo_uint max_lazy,
                                    lzo_uint nice_length,
                                    lzo_uint max_chain,
                                    lzo_uint32 flags )
{
    lzo_bytep op;
    const lzo_bytep ii;
    lzo_uint lit;
    lzo_uint m_len, m_off;
    LZO_COMPRESS_T cc;
    LZO_COMPRESS_T * const c = &cc;
    lzo_swd_p const swd = (lzo_swd_p) wrkmem;
    int r;

    /* sanity check */
#if defined(LZO1X)
    LZO_COMPILE_TIME_ASSERT(LZO1X_999_MEM_COMPRESS >= SIZEOF_LZO_SWD_T)
#elif defined(LZO1Y)
    LZO_COMPILE_TIME_ASSERT(LZO1Y_999_MEM_COMPRESS >= SIZEOF_LZO_SWD_T)
#elif defined(LZO1Z)
    LZO_COMPILE_TIME_ASSERT(LZO1Z_999_MEM_COMPRESS >= SIZEOF_LZO_SWD_T)
#else
#  error
#endif

/* setup parameter defaults */
    /* number of lazy match tries */
    if (try_lazy < 0)
        try_lazy = 1;
    /* reduce lazy match search if we already have a match with this length */
    if (good_length <= 0)
        good_length = 32;
    /* do not try a lazy match if we already have a match with this length */
    if (max_lazy <= 0)
        max_lazy = 32;
    /* stop searching for longer matches than this one */
    if (nice_length <= 0)
        nice_length = 0;
    /* don't search more positions than this */
    if (max_chain <= 0)
        max_chain = SWD_MAX_CHAIN;

    c->init = 0;
    c->ip = c->in = in;
    c->in_end = in + in_len;
    c->out = out;
    c->cb = cb;
    c->m1a_m = c->m1b_m = c->m2_m = c->m3_m = c->m4_m = 0;
    c->lit1_r = c->lit2_r = c->lit3_r = 0;

    op = out;
    ii = c->ip;             /* point to start of literal run */
    lit = 0;
    c->r1_lit = c->r1_m_len = 0;

    r = init_match(c,swd,dict,dict_len,flags);
    if (r != 0)
        return r;
    if (max_chain > 0)
        swd->max_chain = max_chain;
    if (nice_length > 0)
        swd->nice_length = nice_length;

    r = find_match(c,swd,0,0);
    if (r != 0)
        return r;
    while (c->look > 0)
    {
        lzo_uint ahead;
        lzo_uint max_ahead;
        int l1, l2, l3;

        c->codesize = pd(op, out);

        m_len = c->m_len;
        m_off = c->m_off;

        assert(c->bp == c->ip - c->look);
        assert(c->bp >= in);
        if (lit == 0)
            ii = c->bp;
        assert(ii + lit == c->bp);
        assert(swd->b_char == *(c->bp));

        if ( m_len < 2 ||
            (m_len == 2 && (m_off > M1_MAX_OFFSET || lit == 0 || lit >= 4)) ||
#if 1
            /* Do not accept this match for compressed-data compatibility
             * with LZO v1.01 and before
             * [ might be a problem for decompress() and optimize() ]
             */
            (m_len == 2 && op == out) ||
#endif
            (op == out && lit == 0))
        {
            /* a literal */
            m_len = 0;
        }
        else if (m_len == M2_MIN_LEN)
        {
            /* compression ratio improves if we code a literal in some cases */
            if (m_off > MX_MAX_OFFSET && lit >= 4)
                m_len = 0;
        }

        if (m_len == 0)
        {
    /* a literal */
            lit++;
            swd->max_chain = max_chain;
            r = find_match(c,swd,1,0);
            assert(r == 0);
            continue;
        }

    /* a match */
#if defined(SWD_BEST_OFF)
        if (swd->use_best_off)
            better_match(swd,&m_len,&m_off);
#endif
        assert_match(swd,m_len,m_off);



        /* shall we try a lazy match ? */
        ahead = 0;
        if (try_lazy <= 0 || m_len >= max_lazy)
        {
            /* no */
            l1 = 0;
            max_ahead = 0;
        }
        else
        {
            /* yes, try a lazy match */
            l1 = len_of_coded_match(m_len,m_off,lit);
            assert(l1 > 0);
#if 1
            max_ahead = LZO_MIN((lzo_uint)try_lazy, (lzo_uint)l1 - 1);
#else
            max_ahead = LZO_MIN3(try_lazy, l1, m_len - 1);
#endif
        }


        while (ahead < max_ahead && c->look > m_len)
        {
            lzo_int lazy_match_min_gain;

            if (m_len >= good_length)
                swd->max_chain = max_chain >> 2;
            else
                swd->max_chain = max_chain;
            r = find_match(c,swd,1,0);
            ahead++;

            assert(r == 0);
            assert(c->look > 0);
            assert(ii + lit + ahead == c->bp);

#if defined(LZO1Z)
            if (m_off == c->last_m_off && c->m_off != c->last_m_off)
                if (m_len >= M2_MIN_LEN && m_len <= M2_MAX_LEN)
                    c->m_len = 0;
#endif
            if (c->m_len < m_len)
                continue;
#if 1
            if (c->m_len == m_len && c->m_off >= m_off)
                continue;
#endif
#if defined(SWD_BEST_OFF)
            if (swd->use_best_off)
                better_match(swd,&c->m_len,&c->m_off);
#endif
            l2 = len_of_coded_match(c->m_len,c->m_off,lit+ahead);
            if (l2 < 0)
                continue;
#if 0
            if (c->m_len == m_len && l2 >= l1)
                continue;
#endif


#if 1
            /* compressed-data compatibility [see above] */
            l3 = (op == out) ? -1 : len_of_coded_match(ahead,m_off,lit);
#else
            l3 = len_of_coded_match(ahead,m_off,lit);
#endif

            lazy_match_min_gain = min_gain(ahead,lit,lit+ahead,l1,l2,l3);
            if (c->m_len >= m_len + lazy_match_min_gain)
            {
                c->lazy++;
                assert_match(swd,c->m_len,c->m_off);

                if (l3 > 0)
                {
                    /* code previous run */
                    op = code_run(c,op,ii,lit,ahead);
                    lit = 0;
                    /* code shortened match */
                    op = code_match(c,op,ahead,m_off);
                }
                else
                {
                    lit += ahead;
                    assert(ii + lit == c->bp);
                }
                goto lazy_match_done;
            }
        }


        assert(ii + lit + ahead == c->bp);

        /* 1 - code run */
        op = code_run(c,op,ii,lit,m_len);
        lit = 0;

        /* 2 - code match */
        op = code_match(c,op,m_len,m_off);
        swd->max_chain = max_chain;
        r = find_match(c,swd,m_len,1+ahead);
        assert(r == 0);

lazy_match_done: ;
    }


    /* store final run */
    if (lit > 0)
        op = STORE_RUN(c,op,ii,lit);

#if defined(LZO_EOF_CODE)
    *op++ = M4_MARKER | 1;
    *op++ = 0;
    *op++ = 0;
#endif

    c->codesize = pd(op, out);
    assert(c->textsize == in_len);

    *out_len = pd(op, out);

    if (c->cb && c->cb->nprogress)
        (*c->cb->nprogress)(c->cb, c->textsize, c->codesize, 0);

#if 0
    printf("%ld %ld -> %ld  %ld: %ld %ld %ld %ld %ld  %ld: %ld %ld %ld  %ld\n",
        (long) c->textsize, (long) in_len, (long) c->codesize,
        c->match_bytes, c->m1a_m, c->m1b_m, c->m2_m, c->m3_m, c->m4_m,
        c->lit_bytes, c->lit1_r, c->lit2_r, c->lit3_r, c->lazy);
#endif
    assert(c->lit_bytes + c->match_bytes == in_len);

    return LZO_E_OK;
}


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(int)
lzo1x_999_compress_level    ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len,
                                    lzo_callback_p cb,
                                    int compression_level )
{
    static const struct
    {
        int try_lazy;
        lzo_uint good_length;
        lzo_uint max_lazy;
        lzo_uint nice_length;
        lzo_uint max_chain;
        lzo_uint32 flags;
    } c[9] = {
        {   0,   0,   0,   8,    4,   0 },      /* faster compression */
        {   0,   0,   0,  16,    8,   0 },
        {   0,   0,   0,  32,   16,   0 },

        {   1,   4,   4,  16,   16,   0 },
        {   1,   8,  16,  32,   32,   0 },
        {   1,   8,  16, 128,  128,   0 },

        {   2,   8,  32, 128,  256,   0 },
        {   2,  32, 128,   F, 2048,   1 },
        {   2,   F,   F,   F, 4096,   1 }       /* max. compression */
    };

    if (compression_level < 1 || compression_level > 9)
        return LZO_E_ERROR;

    compression_level -= 1;
    return lzo1x_999_compress_internal(in, in_len, out, out_len, wrkmem,
                                       dict, dict_len, cb,
                                       c[compression_level].try_lazy,
                                       c[compression_level].good_length,
                                       c[compression_level].max_lazy,
#if 0
                                       c[compression_level].nice_length,
#else
                                       0,
#endif
                                       c[compression_level].max_chain,
                                       c[compression_level].flags);
}


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(int)
lzo1x_999_compress_dict     ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len )
{
    return lzo1x_999_compress_level(in, in_len, out, out_len, wrkmem,
                                    dict, dict_len, 0, 8);
}

LZO_PUBLIC(int)
lzo1x_999_compress  ( const lzo_bytep in , lzo_uint  in_len,
                            lzo_bytep out, lzo_uintp out_len,
                            lzo_voidp wrkmem )
{
    return lzo1x_999_compress_level(in, in_len, out, out_len, wrkmem,
                                    NULL, 0, (lzo_callback_p) 0, 8);
}


/*
vi:ts=4:et
*/

/* --- lzo_init.c */

/* lzo_init.c -- initialization of the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


/***********************************************************************
// Runtime check of the assumptions about the size of builtin types,
// memory model, byte order and other low-level constructs.
//
// We are really paranoid here - LZO should either fail
// at startup or not at all.
//
// Because of inlining much of these functions evaluates to nothing.
//
// And while many of the tests seem highly obvious and redundant they are
// here to catch compiler/optimizer bugs. Yes, these do exist.
************************************************************************/

#if !defined(__LZO_IN_MINILZO)

#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#include "prepcore_lzo_miniacc.h"

    ACCCHK_ASSERT_IS_SIGNED_T(lzo_int)
    ACCCHK_ASSERT_IS_UNSIGNED_T(lzo_uint)

    ACCCHK_ASSERT_IS_SIGNED_T(lzo_int32)
    ACCCHK_ASSERT_IS_UNSIGNED_T(lzo_uint32)
    ACCCHK_ASSERT((LZO_UINT32_C(1) << (int)(8*sizeof(LZO_UINT32_C(1))-1)) > 0)
    ACCCHK_ASSERT(sizeof(lzo_uint32) >= 4)

#if !defined(__LZO_UINTPTR_T_IS_POINTER)
    ACCCHK_ASSERT_IS_UNSIGNED_T(lzo_uintptr_t)
#endif
    ACCCHK_ASSERT(sizeof(lzo_uintptr_t) >= sizeof(lzo_voidp))

    ACCCHK_ASSERT_IS_UNSIGNED_T(lzo_xint)
    ACCCHK_ASSERT(sizeof(lzo_xint) >= sizeof(lzo_uint32))
    ACCCHK_ASSERT(sizeof(lzo_xint) >= sizeof(lzo_uint))
    ACCCHK_ASSERT(sizeof(lzo_xint) == sizeof(lzo_uint32) || sizeof(lzo_xint) == sizeof(lzo_uint))

#endif
#undef ACCCHK_ASSERT


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(int)
_lzo_config_check(void)
{
    lzo_bool r = 1;
    union { unsigned char c[2*sizeof(lzo_xint)]; lzo_xint l[2]; } u;
    lzo_uintptr_t p;

#if !defined(LZO_CFG_NO_CONFIG_CHECK)
#if defined(LZO_ABI_BIG_ENDIAN)
    u.l[0] = u.l[1] = 0; u.c[sizeof(lzo_xint) - 1] = 128;
    r &= (u.l[0] == 128);
#endif
#if defined(LZO_ABI_LITTLE_ENDIAN)
    u.l[0] = u.l[1] = 0; u.c[0] = 128;
    r &= (u.l[0] == 128);
#endif
#if defined(LZO_UNALIGNED_OK_2)
    p = (lzo_uintptr_t) (const lzo_voidp) &u.c[0];
    u.l[0] = u.l[1] = 0;
    r &= ((* (const lzo_ushortp) (p+1)) == 0);
#endif
#if defined(LZO_UNALIGNED_OK_4)
    p = (lzo_uintptr_t) (const lzo_voidp) &u.c[0];
    u.l[0] = u.l[1] = 0;
    r &= ((* (const lzo_uint32p) (p+1)) == 0);
#endif
#endif

    LZO_UNUSED(u); LZO_UNUSED(p);
    return r == 1 ? LZO_E_OK : LZO_E_ERROR;
}


/***********************************************************************
//
************************************************************************/

int __lzo_init_done = 0;

LZO_PUBLIC(int)
__lzo_init_v2(unsigned v, int s1, int s2, int s3, int s4, int s5,
                          int s6, int s7, int s8, int s9)
{
    int r;

#if defined(__LZO_IN_MINILZO)
#elif (LZO_CC_MSC && ((_MSC_VER) < 700))
#else
#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#define ACCCHK_ASSERT(expr)  LZO_COMPILE_TIME_ASSERT(expr)
#include "prepcore_lzo_miniacc.h"
#endif
#undef ACCCHK_ASSERT

    __lzo_init_done = 1;

    if (v == 0)
        return LZO_E_ERROR;

    r = (s1 == -1 || s1 == (int) sizeof(short)) &&
        (s2 == -1 || s2 == (int) sizeof(int)) &&
        (s3 == -1 || s3 == (int) sizeof(long)) &&
        (s4 == -1 || s4 == (int) sizeof(lzo_uint32)) &&
        (s5 == -1 || s5 == (int) sizeof(lzo_uint)) &&
        (s6 == -1 || s6 == (int) lzo_sizeof_dict_t) &&
        (s7 == -1 || s7 == (int) sizeof(char *)) &&
        (s8 == -1 || s8 == (int) sizeof(lzo_voidp)) &&
        (s9 == -1 || s9 == (int) sizeof(lzo_callback_t));
    if (!r)
        return LZO_E_ERROR;

    r = _lzo_config_check();
    if (r != LZO_E_OK)
        return r;

    return r;
}


/*
vi:ts=4:et
*/

/* --- */
