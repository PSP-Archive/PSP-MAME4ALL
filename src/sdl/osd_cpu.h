/********************************************************************************
*										*
*	Define size independent data types and operations.			*
*										*
*   The following types must be supported by all platforms:			*
*										*
*	UINT8  - Unsigned 8-bit Integer		INT8  - Signed 8-bit integer    *
*	UINT16 - Unsigned 16-bit Integer	INT16 - Signed 16-bit integer   *
*	UINT32 - Unsigned 32-bit Integer	INT32 - Signed 32-bit integer   *
*	UINT64 - Unsigned 64-bit Integer	INT64 - Signed 64-bit integer   *
*										*									   *
********************************************************************************/

#ifndef MAME_OSD_CPU_H
#define MAME_OSD_CPU_H

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned int	UINT32;
typedef signed char 	INT8;
typedef signed short	INT16;
typedef signed int	INT32;

typedef short s16;
#ifndef PSP
typedef int s32;
#endif

typedef signed   long long	INT64;
typedef unsigned long long	UINT64;

/* Combine to 32-bit integers into a 64-bit integer */
#define COMBINE_64_32_32(A,B)     ((((UINT64)(A))<<32) | (B))
#define COMBINE_U64_U32_U32(A,B)  COMBINE_64_32_32(A,B)

/* Return upper 32 bits of a 64-bit integer */
#define HI32_32_64(A)		  (((UINT64)(A)) >> 32)
#define HI32_U32_U64(A)		  HI32_32_64(A)

/* Return lower 32 bits of a 64-bit integer */
#define LO32_32_64(A)		  ((A) & 0xffffffff)
#define LO32_U32_U64(A)		  LO32_32_64(A)

#define DIV_64_64_32(A,B)	  ((A)/(B))
#define DIV_U64_U64_U32(A,B)  ((A)/(UINT32)(B))

#define MOD_32_64_32(A,B)	  ((A)%(B))
#define MOD_U32_U64_U32(A,B)  ((A)%(UINT32)(B))

#define MUL_64_32_32(A,B)	  ((A)*(INT64)(B))
#define MUL_U64_U32_U32(A,B)  ((A)*(UINT64)(UINT32)(B))

#endif	/* defined OSD_CPU_H */
