/*
    This header is a slightly-modified variant of two headers from the
    following source: http://www.lysator.liu.se/(nobg)/c/q8/index.html

    This source code has been placed into the PUBLIC DOMAIN by
    its author: Doug Gwyn (gwyn@arl.mil).
*/

#define _SYS_INT_TYPES_H        /* kludge to override Solaris header */
#define _SYS_INT_LIMITS_H       /* ditto */

/* splice arguments into one token */
#ifdef  __STDC__
#define HL_IN_JOIN1(a,b)    a ## b
#define HL_IN_JOIN2(a,b)    HL_IN_JOIN1(a,b)
#else
#define HL_IN_JOIN1(a)  a
#define HL_IN_JOIN2(a,b)    HL_IN_JOIN1(a)b
#endif

#if defined(__i386) || defined(__amd64)     /* Intel x86, 32-bit/64-bit, GCC/Sun Solaris */

#define HL_IN_CHAR_TYPE /* signed */ char
#define HL_IN_CHAR_SIZE 8
#define HL_IN_SHORT_SIZE    16
#define HL_IN_INT_SIZE  32
#define HL_IN_LONG_SIZE 32
#define HL_IN_PTR_SIZE  32
#define HL_IN_PTR_DIFF_SIZE 32
#define HL_IN_ATOMIC_SIZE   32
#define HL_IN_SIZE_SIZE 32
#define HL_IN_LONG_LONG_SIZE    64
#define HL_IN_LONG_LONG_TYPE    long long
#define HL_IN_LONG_LONG_SUFFIX  LL
#define HL_IN_MAX_SIZE      HL_IN_LONG_LONG_SIZE
#define HL_IN_MAX_TYPE      HL_IN_LONG_LONG_TYPE
#define HL_IN_MAX_SUFFIX    HL_IN_LONG_LONG_SUFFIX

#elif defined(_M_IX86) || defined(_M_X64)   /* Intel x86, 32-bit/64-bit, Microsoft */

#define HL_IN_CHAR_TYPE signed char
#define HL_IN_CHAR_SIZE 8
#define HL_IN_SHORT_SIZE 16
#define HL_IN_INT_SIZE  32
#define HL_IN_LONG_SIZE 32
#define HL_IN_PTR_SIZE  32
#define HL_IN_PTR_DIFF_SIZE 32
#define HL_IN_ATOMIC_SIZE   32
#define HL_IN_SIZE_SIZE 32

#define HL_IN_LONG_LONG_SIZE    64
#define HL_IN_LONG_LONG_TYPE    __int64
#define HL_IN_LONG_LONG_SUFFIX  i64
#define HL_IN_MAX_SIZE      HL_IN_LONG_LONG_SIZE
#define HL_IN_MAX_TYPE      HL_IN_LONG_LONG_TYPE
#define HL_IN_MAX_SUFFIX    HL_IN_LONG_LONG_SUFFIX

#elif   defined(__sparc)        /* SPARC, Sun Solaris */

#define HL_IN_CHAR_TYPE /* signed */ char
#define HL_IN_CHAR_SIZE 8
#define HL_IN_SHORT_SIZE    16
#define HL_IN_INT_SIZE  32
#define HL_IN_ATOMIC_SIZE   32
#ifdef  __sparcv9           /* 64-bit environment */
#define HL_IN_LONG_SIZE 64
#else                   /* 32-bit environment */
#define HL_IN_LONG_SIZE 32
#endif
#define HL_IN_PTR_SIZE      HL_IN_LONG_SIZE
#define HL_IN_PTR_DIFF_SIZE HL_IN_LONG_SIZE
#define HL_IN_SIZE_SIZE     HL_IN_LONG_SIZE
#define HL_IN_LONG_LONG_SIZE    64
#define HL_IN_LONG_LONG_TYPE    long long
#define HL_IN_LONG_LONG_SUFFIX  LL
#define HL_IN_MAX_SIZE      HL_IN_LONG_LONG_SIZE
#define HL_IN_MAX_TYPE      HL_IN_LONG_LONG_TYPE
#define HL_IN_MAX_SUFFIX    HL_IN_LONG_LONG_SUFFIX

#elif   defined(_TMS320C6000)       /* TI TMS320C6xxx DSP, Code Composer */

#define HL_IN_CHAR_TYPE signed char
#define HL_IN_CHAR_SIZE 8
#define HL_IN_SHORT_SIZE    16
#define HL_IN_INT_SIZE  32
#define HL_IN_ATOMIC_SIZE   32
#define HL_IN_LONG_SIZE 40
#define HL_IN_PTR_SIZE  32
#define HL_IN_PTR_DIFF_SIZE 32
#define HL_IN_SIZE_SIZE 32
#if __STDC_VERSION__ >= 199901
#define HL_IN_LONG_LONG_SIZE    64
#define HL_IN_LONG_LONG_TYPE    long long
#define HL_IN_LONG_LONG_SUFFIX  LL
#define HL_IN_MAX_SIZE  HL_IN_LONG_LONG_SIZE
#define HL_IN_MAX_TYPE  HL_IN_LONG_LONG_TYPE
#define HL_IN_MAX_SUFFIX    HL_IN_LONG_LONG_SUFFIX
#else
#define HL_IN_MAX_SIZE  HL_IN_LONG_SIZE
#define HL_IN_MAX_TYPE  long
#define HL_IN_MAX_SUFFIX L
#endif

#elif   defined(__vax)          /* DEC VAX-11 */

#define HL_IN_CHAR_TYPE /* signed */ char
#define HL_IN_CHAR_SIZE 8
#define HL_IN_SHORT_SIZE    16
#define HL_IN_INT_SIZE  32
#define HL_IN_LONG_SIZE 32
#define HL_IN_PTR_SIZE  32
#define HL_IN_PTR_DIFF_SIZE 32
#define HL_IN_ATOMIC_SIZE   32
#define HL_IN_SIZE_SIZE 32
#if __STDC_VERSION__ >= 199901
#define HL_IN_LONG_LONG_SIZE    64
#define HL_IN_LONG_LONG_TYPE    long long
#define HL_IN_LONG_LONG_SUFFIX  LL
#define HL_IN_MAX_SIZE      HL_IN_LONG_LONG_SIZE
#define HL_IN_MAX_TYPE      HL_IN_LONG_LONG_TYPE
#define HL_IN_MAX_SUFFIX    HL_IN_LONG_LONG_SUFFIX
#else
#define HL_IN_MAX_SIZE  32
#define HL_IN_MAX_TYPE  long
#define HL_IN_MAX_SUFFIX    L
#endif

#elif   defined(__pdp11)        /* DEC PDP-11 */

#define HL_IN_CHAR_TYPE /* signed */ char
#define HL_IN_CHAR_SIZE 8
#define HL_IN_SHORT_SIZE    16
#define HL_IN_INT_SIZE  16
#define HL_IN_LONG_SIZE 32
#define HL_IN_PTR_SIZE  16  /* (violates section 5.2.4.1) */
#define HL_IN_PTR_DIFF_SIZE 16  /* (violates section 5.2.4.1) */
#define HL_IN_ATOMIC_SIZE   16
#define HL_IN_SIZE_SIZE 16
#if __STDC_VERSION__ >= 199901
#define HL_IN_LONG_LONG_SIZE    64
#define HL_IN_LONG_LONG_TYPE    long long
#define HL_IN_LONG_LONG_SUFFIX  LL
#define HL_IN_MAX_SIZE      HL_IN_LONG_LONG_SIZE
#define HL_IN_MAX_TYPE      HL_IN_LONG_LONG_TYPE
#define HL_IN_MAX_SUFFIX    HL_IN_LONG_LONG_SUFFIX
#else
#define HL_IN_MAX_SIZE  32
#define HL_IN_MAX_TYPE  long
#define HL_IN_MAX_SUFFIX    L
#endif

#elif   defined(__ORCAC__)      /* Apple IIGS, large memory model */

  /* #pragma    optimize    -1  /* enables all compiler optimizations */

#define HL_IN_CHAR_TYPE signed char
#define HL_IN_CHAR_SIZE 8
#define HL_IN_SHORT_SIZE    16
#define HL_IN_INT_SIZE  16
#define HL_IN_LONG_SIZE 32
#define HL_IN_PTR_SIZE  32
#define HL_IN_PTR_DIFF_SIZE 32
#define HL_IN_ATOMIC_SIZE   16
#define HL_IN_SIZE_SIZE 32      /* XXX -- check */
#if __STDC_VERSION__ >= 199901
#define HL_IN_LONG_LONG_SIZE    64
#define HL_IN_LONG_LONG_TYPE    long long
#define HL_IN_LONG_LONG_SUFFIX  LL
#define HL_IN_MAX_SIZE      HL_IN_LONG_LONG_SIZE
#define HL_IN_MAX_TYPE      HL_IN_LONG_LONG_TYPE
#define HL_IN_MAX_SUFFIX    HL_IN_LONG_LONG_SUFFIX
#else
#define HL_IN_MAX_SIZE  32
#define HL_IN_MAX_TYPE  long
#define HL_IN_MAX_SUFFIX    L
#endif

#else
#error "Unknown CPU type"
#endif

/*  limits for various types    */

/* top signed values */
#define HL_IN_TOP_INT(w)    (~(~0<<(w)-1))
#define HL_IN_TOP_LONG      (~(~0L<<HL_IN_LONG_SIZE-1))
#define HL_IN_TOP_LONG_LONG (~(~HL_IN_JOIN2(0,HL_IN_LONG_LONG_SUFFIX)<<HL_IN_LONG_LONG_SIZE-1))
#define HL_IN_TOP_MAX       (~(~HL_IN_JOIN2(0,HL_IN_MAX_SUFFIX)<<HL_IN_MAX_SIZE-1))

/* upper unsigned values */
#define HL_IN_UPPER_INT(w)      (~0U>>HL_IN_INT_SIZE-(w))
#define HL_IN_UPPER_LONG        (~0UL)
#define HL_IN_UPPER_LONG_LONG   (~HL_IN_JOIN2(0U,HL_IN_LONG_LONG_SUFFIX))
#define HL_IN_UPPER_MAX         (~HL_IN_JOIN2(0U,HL_IN_MAX_SUFFIX))

/* bottom signed values */
#define HL_IN_BOTTOM_INT(w)     (-HL_IN_TOP_INT(w)-1)
#define HL_IN_BOTTOM_LONG       (-HL_IN_TOP_LONG-1)
#define HL_IN_BOTTOM_LONG_LONG  (-HL_IN_TOP_LONG_LONG-1)
#define HL_IN_BOTTOM_MAX        (-HL_IN_TOP_MAX-1)

/* 7.18.1.1 Exact-width integer types. */
#if HL_IN_CHAR_SIZE == 8 && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlS8;
typedef unsigned char       HlU8;
#endif
#if HL_IN_CHAR_SIZE == 16 && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlS16;
typedef unsigned char       HlU16;
#elif   HL_IN_SHORT_SIZE == 16
typedef short               HlS16;
typedef unsigned short      HlU16;
#endif
#if HL_IN_CHAR_SIZE == 32 && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlS32;
typedef unsigned char       HlU32;
#elif   HL_IN_SHORT_SIZE == 32
typedef short               HlS32;
typedef unsigned short      HlU32;
#elif   HL_IN_INT_SIZE == 32
typedef int                 HlS32;
typedef unsigned int        HlU32;
#elif   HL_IN_LONG_SIZE == 32
typedef long                HlS32;
typedef unsigned long       HlU32;
#endif
#if HL_IN_CHAR_SIZE == 64 && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlS64;
typedef unsigned char       HlU64;
#elif   HL_IN_SHORT_SIZE == 64
typedef short               HlS64;
typedef unsigned short      HlU64;
#elif   HL_IN_INT_SIZE == 64
typedef int                 HlS64;
typedef unsigned int        HlU64;
#elif   HL_IN_LONG_SIZE == 64
typedef long                HlS64;
typedef unsigned long       HlU64;
#elif   HL_IN_LONG_LONG_SIZE == 64
typedef HL_IN_LONG_LONG_TYPE            HlS64;
typedef unsigned HL_IN_LONG_LONG_TYPE   HlU64;
#endif

/*
    7.18.1.2    Minimum-width integer types

    The optional 40-bit type is supported by the TMS320C6xxx DSP.

    Prefers type int over char or short char, due to a possible
    problem constructing suitable integer constants for 7.18.4.1.
*/

#if HL_IN_CHAR_SIZE < HL_IN_INT_SIZE && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlLeastS8;
typedef unsigned char       HlLeastU8;
#elif   HL_IN_SHORT_SIZE < HL_IN_INT_SIZE
typedef short               HlLeastS8;
typedef unsigned short      HlLeastU8;
#else
typedef int                 HlLeastS8;
typedef unsigned int        HlLeastU8;
#endif
#if HL_IN_CHAR_SIZE < HL_IN_INT_SIZE && HL_IN_CHAR_SIZE >= 16 && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlLeastS16;
typedef unsigned char       HlLeastU16;
#elif   HL_IN_SHORT_SIZE < HL_IN_INT_SIZE
typedef short               HlLeastS16;
typedef unsigned short      HlLeastU16;
#else
typedef int                 HlLeastS16;
typedef unsigned int        HlLeastU16;
#endif
#if HL_IN_CHAR_SIZE < HL_IN_INT_SIZE && HL_IN_CHAR_SIZE >= 32 && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlLeastS32;
typedef unsigned char       HlLeastU32;
#elif   HL_IN_SHORT_SIZE < HL_IN_INT_SIZE && HL_IN_SHORT_SIZE >= 32
typedef short               HlLeastS32;
typedef unsigned short      HlLeastU32;
#elif   HL_IN_INT_SIZE >= 32
typedef int                 HlLeastS32;
typedef unsigned int        HlLeastU32;
#else
typedef long                HlLeastS32;
typedef unsigned long       HlLeastU32;
#endif
#if HL_IN_CHAR_SIZE < HL_IN_INT_SIZE && HL_IN_CHAR_SIZE >= 64 && defined(HL_IN_CHAR_TYPE)
typedef HL_IN_CHAR_TYPE     HlLeastS64;
typedef unsigned char       HlLeastU64;
#elif   HL_IN_SHORT_SIZE < HL_IN_INT_SIZE && HL_IN_SHORT_SIZE >= 64
typedef short               HlLeastS64;
typedef unsigned short      HlLeastU64;
#elif   HL_IN_INT_SIZE >= 64
typedef int                 HlLeastS64;
typedef unsigned int        HlLeastU64;
#elif   HL_IN_LONG_SIZE >= 64
typedef long                HlLeastS64;
typedef unsigned long       HlLeastU64;
#elif   HL_IN_LONG_LONG_SIZE >= 64          /* (will be 0 if not defined) */
typedef HL_IN_LONG_LONG_TYPE            HlLeastS64;
typedef unsigned HL_IN_LONG_LONG_TYPE   HlLeastU64;
#endif

/*
    7.18.1.4    Integer types capable of holding object pointers

    Theoretically, these tests might not work (e.g., the implementation
    can limit the interconvertible type to one that is not the smallest
    possible), but I'm unaware of any platform where they fail.
*/

#if defined(HL_IN_PTR_SIZE)
#if HL_IN_INT_SIZE >= HL_IN_PTR_SIZE
typedef int                 HlSPtr;
typedef unsigned int        HlUPtr;
#elif   HL_IN_LONG_SIZE >= HL_IN_PTR_SIZE
typedef long                HlSPtr;
typedef unsigned long       HlUPtr;
#elif   HL_IN_LONG_LONG_SIZE >= HL_IN_PTR_SIZE
typedef HL_IN_LONG_LONG_TYPE            HlSPtr;
typedef unsigned HL_IN_LONG_LONG_TYPE   HlUPtr;
#elif   HL_IN_MAX_SIZE >= HL_IN_PTR_SIZE
typedef HL_IN_MAX_TYPE          HlSPtr;
typedef unsigned HL_IN_MAX_TYPE HlUPtr;
#endif
#endif

/*
    7.18.1.5	Greatest-width integer types
*/

typedef HL_IN_MAX_TYPE			HlSMax;
typedef unsigned HL_IN_MAX_TYPE HlUMax;

#if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)

/*
    7.18.2.4    Limits of integer types
                capable of holding object pointers
*/

#if defined(HL_IN_PTR_SIZE)
#if HL_IN_INT_SIZE >= HL_IN_PTR_SIZE
#define HL_SPTR_MAX     HL_IN_TOP_INT(HL_IN_INT_SIZE)
#define HL_SPTR_MIN     HL_IN_BOTTOM_INT(HL_IN_INT_SIZE)
#define HL_UPTR_MAX     HL_IN_UPPER_INT(HL_IN_INT_SIZE)
#elif   HL_IN_LONG_SIZE >= HL_IN_PTR_SIZE
#define HL_SPTR_MAX     HL_IN_TOP_LONG
#define HL_SPTR_MIN     HL_IN_BOTTOM_LONG
#define HL_UPTR_MAX     HL_IN_UPPER_LONG
#elif   HL_IN_LONG_LONG_SIZE >= HL_IN_PTR_SIZE
#define HL_SPTR_MAX     HL_IN_TOP_LONG_LONG
#define HL_SPTR_MIN     HL_IN_BOTTOM_LONG_LONG
#define HL_UPTR_MAX     HL_IN_UPPER_LONG_LONG
#elif   HL_IN_MAX_SIZE >= HL_IN_PTR_SIZE
#define HL_SPTR_MAX     HL_IN_TOP_MAX
#define HL_SPTR_MIN     HL_IN_BOTTOM_MAX
#define HL_UPTR_MAX     HL_IN_UPPER_MAX
#endif
#endif

#if HL_IN_CHAR_SIZE >= HL_IN_SIZE_SIZE
#define HL_SIZE_MAX     HL_IN_UPPER_INT(HL_IN_CHAR_SIZE)
#elif   HL_IN_SHORT_SIZE >= HL_IN_SIZE_SIZE
#define HL_SIZE_MAX     HL_IN_UPPER_INT(HL_IN_SHORT_SIZE)
#elif   HL_IN_INT_SIZE >= HL_IN_SIZE_SIZE
#define HL_SIZE_MAX     HL_IN_UPPER_INT(HL_IN_INT_SIZE)
#elif   HL_IN_LONG_SIZE >= HL_IN_SIZE_SIZE
#define HL_SIZE_MAX     HL_IN_UPPER_LONG
#elif   HL_IN_LONG_LONG_SIZE >= HL_IN_SIZE_SIZE
#define HL_SIZE_MAX     HL_IN_UPPER_LONG_LONG
#else
#define HL_SIZE_MAX     HL_IN_UPPER_MAX
#endif

/*
    7.18.2.5	Limits of greatest-width integer types
*/

#define	HL_SMAX_MAX HL_IN_TOP_MAX
#define	HL_UMAX_MAX HL_IN_UPPER_MAX

#endif  /* !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS) */
