/*
     Definitions for the CPU-Modules
*/

#ifndef __m68000defs__
#define __m68000defs__


#include <stdlib.h>
#include "../driver.h"
#include "../memory.h"

#ifdef DREAMCAST
#ifdef INLINE
#undef INLINE
#endif
#define INLINE static __inline__
#endif

#ifndef INLINE
#define INLINE static __inline
#endif


#ifdef __MWERKS__
#pragma require_prototypes off
#endif



typedef signed char	BYTE;
typedef unsigned char	UBYTE;
typedef unsigned short	UWORD;
typedef short		WORD;
typedef unsigned int	ULONG;
typedef int		LONG;
typedef unsigned int	CPTR;

/****************************************************************************/
/* Define a MC68K word. Upper bytes are always zero			    */
/****************************************************************************/
typedef union
{
/*
#ifdef __128BIT__
 #ifdef LSB_FIRST
   struct { UBYTE l,h,h2,h3,h4,h5,h6,h7, h8,h9,h10,h11,h12,h13,h14,h15; } B;
   struct { UWORD l,h,h2,h3,h4,h5,h6,h7; } W;
   ULONG D;
 #else
   struct { UBYTE h15,h14,h13,h12,h11,h10,h9,h8, h7,h6,h5,h4,h3,h2,h,l; } B;
   struct { UWORD h7,h6,h5,h4,h3,h2,h,l; } W;
   ULONG D;
 #endif
#elif __64BIT__
 #ifdef LSB_FIRST
   struct { UBYTE l,h,h2,h3,h4,h5,h6,h7; } B;
   struct { UWORD l,h,h2,h3; } W;
   ULONG D;
 #else
   struct { UBYTE h7,h6,h5,h4,h3,h2,h,l; } B;
   struct { UWORD h3,h2,h,l; } W;
   ULONG D;
 #endif
#else
*/
 #ifdef LSB_FIRST
   struct { UBYTE l,h,h2,h3; } B;
   struct { UWORD l,h; } W;
   ULONG D;
 #else
   struct { UBYTE h3,h2,h,l; } B;
   struct { UWORD h,l; } W;
   ULONG D;
 #endif
/*#endif*/
} pair68000;


extern void Exception(int nr, CPTR oldpc);

typedef void cpuop_func(void);
extern cpuop_func *cpufunctbl[65536];


typedef char flagtype;
#ifndef __WATCOMC__
#define READ_MEML(a,b) asm ("mov (%%esi),%%eax \n\t bswap %%eax \n\t" :"=a" (b) :"S" (a))
#define READ_MEMW(a,b) asm ("mov (%%esi),%%ax\n\t  xchg %%al,%%ah" :"=a" (b) : "S" (a))
#endif
#ifdef __WATCOMC__
LONG wat_readmeml(void *a);
#pragma aux wat_readmeml=\
       "mov eax,[esi]"\
       "bswap eax    "\
       parm [esi] \
       value [eax];
#define READ_MEML(a,b) b=wat_readmeml(a)
WORD wat_readmemw(void *a);
#pragma aux wat_readmemw=\
       "mov ax,[esi]"\
       "xchg al,ah    "\
       parm [esi] \
       value [ax];
#define READ_MEMW(a,b) b=wat_readmemw(a)
#endif

#define get_byte(a) cpu_readmem24((a)&0xffffff)
#define get_word(a) cpu_readmem24_word((a)&0xffffff)
#define get_long(a) cpu_readmem24_dword((a)&0xffffff)
#define put_byte(a,b) cpu_writemem24((a)&0xffffff,b)
#define put_word(a,b) cpu_writemem24_word((a)&0xffffff,b)
#define put_long(a,b) cpu_writemem24_dword((a)&0xffffff,b)

/***************************************/
/* First block is for C 68000 emulator */
/***************************************/

#ifndef A68KEM

union flagu {
    struct {
        char c;
        char v;
        char z;
        char n;
    } flags;
    struct {
        unsigned short vc;
        unsigned short nz;
    } quickclear;
    ULONG longflags;
};

#define CLEARVC regflags.quickclear.vc=0;
#define CLEARFLGS regflags.longflags=0;

#define CTRUE 0x01
#define VTRUE 0x01
#define ZTRUE 0x01
#define NTRUE 0x01
#define XTRUE 0x01

extern int areg_byteinc[];
extern int movem_index1[256];
extern int movem_index2[256];
extern int movem_next[256];
extern int imm8_table[];
extern UBYTE *actadr;

extern int MC68000_ICount ;

typedef struct
{
		pair68000  d[8];
            CPTR  a[8],usp,isp,msp;
            ULONG pc;
            UWORD sr;
            flagtype t1;
            flagtype t0;
            flagtype s;
            flagtype m;
            flagtype x;
            flagtype stopped;
            int intmask;
            ULONG vbr,sfc,dfc;
            double fp[8];
            ULONG fpcr,fpsr,fpiar;
} m68k_regstruct;

extern m68k_regstruct m68k_regs, lastint_m68k_regs;


extern union flagu intel_flag_lookup[256];
extern union flagu regflags;
extern UBYTE aslmask_ubyte[];
extern UWORD aslmask_uword[];
extern ULONG aslmask_ulong[];

#define ZFLG (regflags.flags.z)
#define NFLG (regflags.flags.n)
#define CFLG (regflags.flags.c)
#define VFLG (regflags.flags.v)

#ifdef ASM_MEMORY
INLINE UWORD nextiword_opcode(void)
{
        asm(" \
                movzwl  _m68k_regs+88,%ecx \
                movzbl  _m68k_regs+90,%ebx \
                addl    $2,_m68k_regs+88 \
                movl    _MRAM(,%ebx,4),%edx \
                movb    (%ecx, %edx),%bh \
                movb    1(%ecx, %edx),%bl \
        ");
}
#endif

INLINE UWORD nextiword(void)
{
    m68k_regs.pc+=2;
    return (cpu_readop16(m68k_regs.pc-2));
}

INLINE ULONG nextilong(void)
{
    m68k_regs.pc+=4;
    return ((cpu_readop16(m68k_regs.pc-4)<<16) | cpu_readop16(m68k_regs.pc-2));
}

INLINE void m68k_setpc(CPTR newpc)
{
    m68k_regs.pc = newpc;
    change_pc24(m68k_regs.pc&0xffffff);
}

INLINE CPTR m68k_getpc(void)
{
    return m68k_regs.pc;
}

INLINE ULONG get_disp_ea (ULONG base)
{
   UWORD dp = nextiword();
        int reg = (dp >> 12) & 7;
	LONG regd = dp & 0x8000 ? m68k_regs.a[reg] : m68k_regs.d[reg].D;
        if ((dp & 0x800) == 0)
            regd = (LONG)(WORD)regd;
        return base + (BYTE)(dp) + regd;
}

INLINE int cctrue(const int cc)
{
            switch(cc){
              case 0: return 1;                       /* T */
              case 1: return 0;                       /* F */
              case 2: return !CFLG && !ZFLG;          /* HI */
              case 3: return CFLG || ZFLG;            /* LS */
              case 4: return !CFLG;                   /* CC */
              case 5: return CFLG;                    /* CS */
              case 6: return !ZFLG;                   /* NE */
              case 7: return ZFLG;                    /* EQ */
              case 8: return !VFLG;                   /* VC */
              case 9: return VFLG;                    /* VS */
              case 10:return !NFLG;                   /* PL */
              case 11:return NFLG;                    /* MI */
              case 12:return NFLG == VFLG;            /* GE */
              case 13:return NFLG != VFLG;            /* LT */
              case 14:return !ZFLG && (NFLG == VFLG); /* GT */
              case 15:return ZFLG || (NFLG != VFLG);  /* LE */
             }
             abort();
             return 0;
}

INLINE void MakeSR(void)
{

	int sr ;

	sr = m68k_regs.t1 ;
	sr <<= 1 ;
	sr |= m68k_regs.t0 ;
	sr <<= 1 ;
	sr |= m68k_regs.s ;
	sr <<= 1 ;
	sr |= m68k_regs.m ;
	sr <<= 4 ;
	sr |= m68k_regs.intmask ;
	sr <<= 4 ;
	sr |= m68k_regs.x ;
	sr <<= 1 ;
	sr |= NFLG ;
	sr <<= 1 ;
	sr |= ZFLG ;
	sr <<= 1 ;
	sr |= VFLG ;
	sr <<= 1 ;
	sr |= CFLG ;

	m68k_regs.sr = sr ;


/*
     m68k_regs.sr = ((m68k_regs.t1 << 15) | (m68k_regs.t0 << 14)
      | (m68k_regs.s << 13) | (m68k_regs.m << 12) | (m68k_regs.intmask << 8)
      | (m68k_regs.x << 4) | (NFLG << 3) | (ZFLG << 2) | (VFLG << 1)
      |  CFLG);
*/
}

INLINE void MakeFromSR(void)
{
  /*  int oldm = m68k_regs.m; */
	int olds = m68k_regs.s;


	int sr = m68k_regs.sr ;

	CFLG = sr & 1 ;
	sr >>= 1 ;
	VFLG = sr & 1 ;
	sr >>= 1 ;
	ZFLG = sr & 1 ;
	sr >>= 1 ;
	NFLG = sr & 1 ;
	sr >>= 1 ;
	m68k_regs.x = sr & 1 ;
	sr >>= 4 ;
	m68k_regs.intmask = sr & 7 ;
	sr >>= 4 ;
	m68k_regs.m = sr & 1 ;
	sr >>= 1 ;
	m68k_regs.s = sr & 1 ;
	sr >>= 1 ;
	m68k_regs.t0 = sr & 1 ;
	sr >>= 1 ;
	m68k_regs.t1 = sr & 1 ;

/*
    m68k_regs.t1 = (m68k_regs.sr >> 15) & 1;
    m68k_regs.t0 = (m68k_regs.sr >> 14) & 1;
    m68k_regs.s = (m68k_regs.sr >> 13) & 1;
    m68k_regs.m = (m68k_regs.sr >> 12) & 1;
    m68k_regs.intmask = (m68k_regs.sr >> 8) & 7;
    m68k_regs.x = (m68k_regs.sr >> 4) & 1;
    NFLG = (m68k_regs.sr >> 3) & 1;
    ZFLG = (m68k_regs.sr >> 2) & 1;
    VFLG = (m68k_regs.sr >> 1) & 1;
    CFLG = m68k_regs.sr & 1;
*/

    if (olds != m68k_regs.s) {
       if (olds) {
         m68k_regs.isp = m68k_regs.a[7];
         m68k_regs.a[7] = m68k_regs.usp;
        } else {
           m68k_regs.usp = m68k_regs.a[7];
           m68k_regs.a[7] = m68k_regs.isp;
        }
     }

}

extern int opcode ;

		/************************/
#else 	/* Structures for 68KEM */
		/************************/

/*	Assembler Engine Register Structure */

typedef struct
{
    ULONG d[8];             /* 0x0004 8 Data registers */
	CPTR  a[8];             /* 0x0024 8 Address registers */

    CPTR  usp;              /* 0x0044 Stack registers (They will overlap over reg_a7) */
    CPTR  isp;              /* 0x0048 */

    ULONG sr_high;     		/* 0x004C System registers */
    ULONG ccr;              /* 0x0050 CCR in Intel Format */
    ULONG x_carry;			/* 0x0054 Extended Carry */

    ULONG pc;            	/* 0x0058 Program Counter */

    ULONG IRQ_level;        /* 0x005C IRQ level you want the MC68K process (0=None)  */

    /* Backward compatible with C emulator - Only set in Debug compile */

    UWORD sr;

} m68k_regstruct;

#endif

#endif
