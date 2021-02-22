/*
     Definitions for the CPU-Modules
*/

#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)

#ifndef __musashi000defs__
#define __musashi000defs__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
/* Define a MC_MUSASHIK word. Upper bytes are always zero			    */
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

extern int MC_MUSASHI000_ICount;

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
} musashik_regstruct;

extern musashik_regstruct musashik_regs, lastint_musashik_regs;


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
                movzwl  _musashik_regs+88,%ecx \
                movzbl  _musashik_regs+90,%ebx \
                addl    $2,_musashik_regs+88 \
                movl    _MRAM(,%ebx,4),%edx \
                movb    (%ecx, %edx),%bh \
                movb    1(%ecx, %edx),%bl \
        ");
}
#endif

INLINE UWORD nextiword(void)
{
    musashik_regs.pc+=2;
    return (cpu_readop16(musashik_regs.pc-2));
}

INLINE ULONG nextilong(void)
{
    musashik_regs.pc+=4;
    return ((cpu_readop16(musashik_regs.pc-4)<<16) | cpu_readop16(musashik_regs.pc-2));
}

INLINE void musashik_setpc(CPTR newpc)
{
    musashik_regs.pc = newpc;
    change_pc24(musashik_regs.pc&0xffffff);
}

INLINE CPTR musashik_getpc(void)
{
    return musashik_regs.pc;
}

INLINE ULONG get_disp_ea (ULONG base)
{
   UWORD dp = nextiword();
        int reg = (dp >> 12) & 7;
	LONG regd = dp & 0x8000 ? musashik_regs.a[reg] : musashik_regs.d[reg].D;
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

	sr = musashik_regs.t1 ;
	sr <<= 1 ;
	sr |= musashik_regs.t0 ;
	sr <<= 1 ;
	sr |= musashik_regs.s ;
	sr <<= 1 ;
	sr |= musashik_regs.m ;
	sr <<= 4 ;
	sr |= musashik_regs.intmask ;
	sr <<= 4 ;
	sr |= musashik_regs.x ;
	sr <<= 1 ;
	sr |= NFLG ;
	sr <<= 1 ;
	sr |= ZFLG ;
	sr <<= 1 ;
	sr |= VFLG ;
	sr <<= 1 ;
	sr |= CFLG ;

	musashik_regs.sr = sr ;


/*
     musashik_regs.sr = ((musashik_regs.t1 << 15) | (musashik_regs.t0 << 14)
      | (musashik_regs.s << 13) | (musashik_regs.m << 12) | (musashik_regs.intmask << 8)
      | (musashik_regs.x << 4) | (NFLG << 3) | (ZFLG << 2) | (VFLG << 1)
      |  CFLG);
*/
}

INLINE void MakeFromSR(void)
{
  /*  int oldm = musashik_regs.m; */
	int olds = musashik_regs.s;


	int sr = musashik_regs.sr ;

	CFLG = sr & 1 ;
	sr >>= 1 ;
	VFLG = sr & 1 ;
	sr >>= 1 ;
	ZFLG = sr & 1 ;
	sr >>= 1 ;
	NFLG = sr & 1 ;
	sr >>= 1 ;
	musashik_regs.x = sr & 1 ;
	sr >>= 4 ;
	musashik_regs.intmask = sr & 7 ;
	sr >>= 4 ;
	musashik_regs.m = sr & 1 ;
	sr >>= 1 ;
	musashik_regs.s = sr & 1 ;
	sr >>= 1 ;
	musashik_regs.t0 = sr & 1 ;
	sr >>= 1 ;
	musashik_regs.t1 = sr & 1 ;

/*
    musashik_regs.t1 = (musashik_regs.sr >> 15) & 1;
    musashik_regs.t0 = (musashik_regs.sr >> 14) & 1;
    musashik_regs.s = (musashik_regs.sr >> 13) & 1;
    musashik_regs.m = (musashik_regs.sr >> 12) & 1;
    musashik_regs.intmask = (musashik_regs.sr >> 8) & 7;
    musashik_regs.x = (musashik_regs.sr >> 4) & 1;
    NFLG = (musashik_regs.sr >> 3) & 1;
    ZFLG = (musashik_regs.sr >> 2) & 1;
    VFLG = (musashik_regs.sr >> 1) & 1;
    CFLG = musashik_regs.sr & 1;
*/

    if (olds != musashik_regs.s) {
       if (olds) {
         musashik_regs.isp = musashik_regs.a[7];
         musashik_regs.a[7] = musashik_regs.usp;
        } else {
           musashik_regs.usp = musashik_regs.a[7];
           musashik_regs.a[7] = musashik_regs.isp;
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

    ULONG IRQ_level;        /* 0x005C IRQ level you want the MC_MUSASHIK process (0=None)  */

    /* Backward compatible with C emulator - Only set in Debug compile */

    UWORD sr;

} musashik_regstruct;

#endif

#endif
#ifndef __MCcputbl__
#define __MCcputbl__

extern cpuop_func op_0;
extern cpuop_func op_10;
extern cpuop_func op_18;
extern cpuop_func op_20;
extern cpuop_func op_28;
extern cpuop_func op_30;
extern cpuop_func op_38;
extern cpuop_func op_39;
extern cpuop_func op_3c;
extern cpuop_func op_40;
extern cpuop_func op_50;
extern cpuop_func op_58;
extern cpuop_func op_60;
extern cpuop_func op_68;
extern cpuop_func op_70;
extern cpuop_func op_78;
extern cpuop_func op_79;
extern cpuop_func op_7c;
extern cpuop_func op_80;
extern cpuop_func op_90;
extern cpuop_func op_98;
extern cpuop_func op_a0;
extern cpuop_func op_a8;
extern cpuop_func op_b0;
extern cpuop_func op_b8;
extern cpuop_func op_b9;
extern cpuop_func op_100;
extern cpuop_func op_108;
extern cpuop_func op_110;
extern cpuop_func op_118;
extern cpuop_func op_120;
extern cpuop_func op_128;
extern cpuop_func op_130;
extern cpuop_func op_138;
extern cpuop_func op_139;
extern cpuop_func op_13a;
extern cpuop_func op_13b;
extern cpuop_func op_13c;
extern cpuop_func op_140;
extern cpuop_func op_148;
extern cpuop_func op_150;
extern cpuop_func op_158;
extern cpuop_func op_160;
extern cpuop_func op_168;
extern cpuop_func op_170;
extern cpuop_func op_178;
extern cpuop_func op_179;
extern cpuop_func op_17a;
extern cpuop_func op_17b;
extern cpuop_func op_180;
extern cpuop_func op_188;
extern cpuop_func op_190;
extern cpuop_func op_198;
extern cpuop_func op_1a0;
extern cpuop_func op_1a8;
extern cpuop_func op_1b0;
extern cpuop_func op_1b8;
extern cpuop_func op_1b9;
extern cpuop_func op_1ba;
extern cpuop_func op_1bb;
extern cpuop_func op_1c0;
extern cpuop_func op_1c8;
extern cpuop_func op_1d0;
extern cpuop_func op_1d8;
extern cpuop_func op_1e0;
extern cpuop_func op_1e8;
extern cpuop_func op_1f0;
extern cpuop_func op_1f8;
extern cpuop_func op_1f9;
extern cpuop_func op_1fa;
extern cpuop_func op_1fb;
extern cpuop_func op_200;
extern cpuop_func op_210;
extern cpuop_func op_218;
extern cpuop_func op_220;
extern cpuop_func op_228;
extern cpuop_func op_230;
extern cpuop_func op_238;
extern cpuop_func op_239;
extern cpuop_func op_23c;
extern cpuop_func op_240;
extern cpuop_func op_250;
extern cpuop_func op_258;
extern cpuop_func op_260;
extern cpuop_func op_268;
extern cpuop_func op_270;
extern cpuop_func op_278;
extern cpuop_func op_279;
extern cpuop_func op_27c;
extern cpuop_func op_280;
extern cpuop_func op_290;
extern cpuop_func op_298;
extern cpuop_func op_2a0;
extern cpuop_func op_2a8;
extern cpuop_func op_2b0;
extern cpuop_func op_2b8;
extern cpuop_func op_2b9;
extern cpuop_func op_400;
extern cpuop_func op_410;
extern cpuop_func op_418;
extern cpuop_func op_420;
extern cpuop_func op_428;
extern cpuop_func op_430;
extern cpuop_func op_438;
extern cpuop_func op_439;
extern cpuop_func op_440;
extern cpuop_func op_450;
extern cpuop_func op_458;
extern cpuop_func op_460;
extern cpuop_func op_468;
extern cpuop_func op_470;
extern cpuop_func op_478;
extern cpuop_func op_479;
extern cpuop_func op_480;
extern cpuop_func op_490;
extern cpuop_func op_498;
extern cpuop_func op_4a0;
extern cpuop_func op_4a8;
extern cpuop_func op_4b0;
extern cpuop_func op_4b8;
extern cpuop_func op_4b9;
extern cpuop_func op_600;
extern cpuop_func op_610;
extern cpuop_func op_618;
extern cpuop_func op_620;
extern cpuop_func op_628;
extern cpuop_func op_630;
extern cpuop_func op_638;
extern cpuop_func op_639;
extern cpuop_func op_640;
extern cpuop_func op_650;
extern cpuop_func op_658;
extern cpuop_func op_660;
extern cpuop_func op_668;
extern cpuop_func op_670;
extern cpuop_func op_678;
extern cpuop_func op_679;
extern cpuop_func op_680;
extern cpuop_func op_690;
extern cpuop_func op_698;
extern cpuop_func op_6a0;
extern cpuop_func op_6a8;
extern cpuop_func op_6b0;
extern cpuop_func op_6b8;
extern cpuop_func op_6b9;
extern cpuop_func op_800;
extern cpuop_func op_810;
extern cpuop_func op_818;
extern cpuop_func op_820;
extern cpuop_func op_828;
extern cpuop_func op_830;
extern cpuop_func op_838;
extern cpuop_func op_839;
extern cpuop_func op_83a;
extern cpuop_func op_83b;
extern cpuop_func op_83c;
extern cpuop_func op_840;
extern cpuop_func op_850;
extern cpuop_func op_858;
extern cpuop_func op_860;
extern cpuop_func op_868;
extern cpuop_func op_870;
extern cpuop_func op_878;
extern cpuop_func op_879;
extern cpuop_func op_87a;
extern cpuop_func op_87b;
extern cpuop_func op_880;
extern cpuop_func op_890;
extern cpuop_func op_898;
extern cpuop_func op_8a0;
extern cpuop_func op_8a8;
extern cpuop_func op_8b0;
extern cpuop_func op_8b8;
extern cpuop_func op_8b9;
extern cpuop_func op_8ba;
extern cpuop_func op_8bb;
extern cpuop_func op_8c0;
extern cpuop_func op_8d0;
extern cpuop_func op_8d8;
extern cpuop_func op_8e0;
extern cpuop_func op_8e8;
extern cpuop_func op_8f0;
extern cpuop_func op_8f8;
extern cpuop_func op_8f9;
extern cpuop_func op_8fa;
extern cpuop_func op_8fb;
extern cpuop_func op_a00;
extern cpuop_func op_a10;
extern cpuop_func op_a18;
extern cpuop_func op_a20;
extern cpuop_func op_a28;
extern cpuop_func op_a30;
extern cpuop_func op_a38;
extern cpuop_func op_a39;
extern cpuop_func op_a3c;
extern cpuop_func op_a40;
extern cpuop_func op_a50;
extern cpuop_func op_a58;
extern cpuop_func op_a60;
extern cpuop_func op_a68;
extern cpuop_func op_a70;
extern cpuop_func op_a78;
extern cpuop_func op_a79;
extern cpuop_func op_a7c;
extern cpuop_func op_a80;
extern cpuop_func op_a90;
extern cpuop_func op_a98;
extern cpuop_func op_aa0;
extern cpuop_func op_aa8;
extern cpuop_func op_ab0;
extern cpuop_func op_ab8;
extern cpuop_func op_ab9;
extern cpuop_func op_c00;
extern cpuop_func op_c10;
extern cpuop_func op_c18;
extern cpuop_func op_c20;
extern cpuop_func op_c28;
extern cpuop_func op_c30;
extern cpuop_func op_c38;
extern cpuop_func op_c39;
extern cpuop_func op_c3a;
extern cpuop_func op_c3b;
extern cpuop_func op_c40;
extern cpuop_func op_c50;
extern cpuop_func op_c58;
extern cpuop_func op_c60;
extern cpuop_func op_c68;
extern cpuop_func op_c70;
extern cpuop_func op_c78;
extern cpuop_func op_c79;
extern cpuop_func op_c7a;
extern cpuop_func op_c7b;
extern cpuop_func op_c80;
extern cpuop_func op_c90;
extern cpuop_func op_c98;
extern cpuop_func op_ca0;
extern cpuop_func op_ca8;
extern cpuop_func op_cb0;
extern cpuop_func op_cb8;
extern cpuop_func op_cb9;
extern cpuop_func op_cba;
extern cpuop_func op_cbb;
extern cpuop_func op_1000;
extern cpuop_func op_1010;
extern cpuop_func op_1018;
extern cpuop_func op_1020;
extern cpuop_func op_1028;
extern cpuop_func op_1030;
extern cpuop_func op_1038;
extern cpuop_func op_1039;
extern cpuop_func op_103a;
extern cpuop_func op_103b;
extern cpuop_func op_103c;
extern cpuop_func op_1080;
extern cpuop_func op_1090;
extern cpuop_func op_1098;
extern cpuop_func op_10a0;
extern cpuop_func op_10a8;
extern cpuop_func op_10b0;
extern cpuop_func op_10b8;
extern cpuop_func op_10b9;
extern cpuop_func op_10ba;
extern cpuop_func op_10bb;
extern cpuop_func op_10bc;
extern cpuop_func op_10c0;
extern cpuop_func op_10d0;
extern cpuop_func op_10d8;
extern cpuop_func op_10e0;
extern cpuop_func op_10e8;
extern cpuop_func op_10f0;
extern cpuop_func op_10f8;
extern cpuop_func op_10f9;
extern cpuop_func op_10fa;
extern cpuop_func op_10fb;
extern cpuop_func op_10fc;
extern cpuop_func op_1100;
extern cpuop_func op_1110;
extern cpuop_func op_1118;
extern cpuop_func op_1120;
extern cpuop_func op_1128;
extern cpuop_func op_1130;
extern cpuop_func op_1138;
extern cpuop_func op_1139;
extern cpuop_func op_113a;
extern cpuop_func op_113b;
extern cpuop_func op_113c;
extern cpuop_func op_1140;
extern cpuop_func op_1150;
extern cpuop_func op_1158;
extern cpuop_func op_1160;
extern cpuop_func op_1168;
extern cpuop_func op_1170;
extern cpuop_func op_1178;
extern cpuop_func op_1179;
extern cpuop_func op_117a;
extern cpuop_func op_117b;
extern cpuop_func op_117c;
extern cpuop_func op_1180;
extern cpuop_func op_1190;
extern cpuop_func op_1198;
extern cpuop_func op_11a0;
extern cpuop_func op_11a8;
extern cpuop_func op_11b0;
extern cpuop_func op_11b8;
extern cpuop_func op_11b9;
extern cpuop_func op_11ba;
extern cpuop_func op_11bb;
extern cpuop_func op_11bc;
extern cpuop_func op_11c0;
extern cpuop_func op_11d0;
extern cpuop_func op_11d8;
extern cpuop_func op_11e0;
extern cpuop_func op_11e8;
extern cpuop_func op_11f0;
extern cpuop_func op_11f8;
extern cpuop_func op_11f9;
extern cpuop_func op_11fa;
extern cpuop_func op_11fb;
extern cpuop_func op_11fc;
extern cpuop_func op_13c0;
extern cpuop_func op_13d0;
extern cpuop_func op_13d8;
extern cpuop_func op_13e0;
extern cpuop_func op_13e8;
extern cpuop_func op_13f0;
extern cpuop_func op_13f8;
extern cpuop_func op_13f9;
extern cpuop_func op_13fa;
extern cpuop_func op_13fb;
extern cpuop_func op_13fc;
extern cpuop_func op_2000;
extern cpuop_func op_2008;
extern cpuop_func op_2010;
extern cpuop_func op_2018;
extern cpuop_func op_2020;
extern cpuop_func op_2028;
extern cpuop_func op_2030;
extern cpuop_func op_2038;
extern cpuop_func op_2039;
extern cpuop_func op_203a;
extern cpuop_func op_203b;
extern cpuop_func op_203c;
extern cpuop_func op_2040;
extern cpuop_func op_2048;
extern cpuop_func op_2050;
extern cpuop_func op_2058;
extern cpuop_func op_2060;
extern cpuop_func op_2068;
extern cpuop_func op_2070;
extern cpuop_func op_2078;
extern cpuop_func op_2079;
extern cpuop_func op_207a;
extern cpuop_func op_207b;
extern cpuop_func op_207c;
extern cpuop_func op_2080;
extern cpuop_func op_2088;
extern cpuop_func op_2090;
extern cpuop_func op_2098;
extern cpuop_func op_20a0;
extern cpuop_func op_20a8;
extern cpuop_func op_20b0;
extern cpuop_func op_20b8;
extern cpuop_func op_20b9;
extern cpuop_func op_20ba;
extern cpuop_func op_20bb;
extern cpuop_func op_20bc;
extern cpuop_func op_20c0;
extern cpuop_func op_20c8;
extern cpuop_func op_20d0;
extern cpuop_func op_20d8;
extern cpuop_func op_20e0;
extern cpuop_func op_20e8;
extern cpuop_func op_20f0;
extern cpuop_func op_20f8;
extern cpuop_func op_20f9;
extern cpuop_func op_20fa;
extern cpuop_func op_20fb;
extern cpuop_func op_20fc;
extern cpuop_func op_2100;
extern cpuop_func op_2108;
extern cpuop_func op_2110;
extern cpuop_func op_2118;
extern cpuop_func op_2120;
extern cpuop_func op_2128;
extern cpuop_func op_2130;
extern cpuop_func op_2138;
extern cpuop_func op_2139;
extern cpuop_func op_213a;
extern cpuop_func op_213b;
extern cpuop_func op_213c;
extern cpuop_func op_2140;
extern cpuop_func op_2148;
extern cpuop_func op_2150;
extern cpuop_func op_2158;
extern cpuop_func op_2160;
extern cpuop_func op_2168;
extern cpuop_func op_2170;
extern cpuop_func op_2178;
extern cpuop_func op_2179;
extern cpuop_func op_217a;
extern cpuop_func op_217b;
extern cpuop_func op_217c;
extern cpuop_func op_2180;
extern cpuop_func op_2188;
extern cpuop_func op_2190;
extern cpuop_func op_2198;
extern cpuop_func op_21a0;
extern cpuop_func op_21a8;
extern cpuop_func op_21b0;
extern cpuop_func op_21b8;
extern cpuop_func op_21b9;
extern cpuop_func op_21ba;
extern cpuop_func op_21bb;
extern cpuop_func op_21bc;
extern cpuop_func op_21c0;
extern cpuop_func op_21c8;
extern cpuop_func op_21d0;
extern cpuop_func op_21d8;
extern cpuop_func op_21e0;
extern cpuop_func op_21e8;
extern cpuop_func op_21f0;
extern cpuop_func op_21f8;
extern cpuop_func op_21f9;
extern cpuop_func op_21fa;
extern cpuop_func op_21fb;
extern cpuop_func op_21fc;
extern cpuop_func op_23c0;
extern cpuop_func op_23c8;
extern cpuop_func op_23d0;
extern cpuop_func op_23d8;
extern cpuop_func op_23e0;
extern cpuop_func op_23e8;
extern cpuop_func op_23f0;
extern cpuop_func op_23f8;
extern cpuop_func op_23f9;
extern cpuop_func op_23fa;
extern cpuop_func op_23fb;
extern cpuop_func op_23fc;
extern cpuop_func op_3000;
extern cpuop_func op_3008;
extern cpuop_func op_3010;
extern cpuop_func op_3018;
extern cpuop_func op_3020;
extern cpuop_func op_3028;
extern cpuop_func op_3030;
extern cpuop_func op_3038;
extern cpuop_func op_3039;
extern cpuop_func op_303a;
extern cpuop_func op_303b;
extern cpuop_func op_303c;
extern cpuop_func op_3040;
extern cpuop_func op_3048;
extern cpuop_func op_3050;
extern cpuop_func op_3058;
extern cpuop_func op_3060;
extern cpuop_func op_3068;
extern cpuop_func op_3070;
extern cpuop_func op_3078;
extern cpuop_func op_3079;
extern cpuop_func op_307a;
extern cpuop_func op_307b;
extern cpuop_func op_307c;
extern cpuop_func op_3080;
extern cpuop_func op_3088;
extern cpuop_func op_3090;
extern cpuop_func op_3098;
extern cpuop_func op_30a0;
extern cpuop_func op_30a8;
extern cpuop_func op_30b0;
extern cpuop_func op_30b8;
extern cpuop_func op_30b9;
extern cpuop_func op_30ba;
extern cpuop_func op_30bb;
extern cpuop_func op_30bc;
extern cpuop_func op_30c0;
extern cpuop_func op_30c8;
extern cpuop_func op_30d0;
extern cpuop_func op_30d8;
extern cpuop_func op_30e0;
extern cpuop_func op_30e8;
extern cpuop_func op_30f0;
extern cpuop_func op_30f8;
extern cpuop_func op_30f9;
extern cpuop_func op_30fa;
extern cpuop_func op_30fb;
extern cpuop_func op_30fc;
extern cpuop_func op_3100;
extern cpuop_func op_3108;
extern cpuop_func op_3110;
extern cpuop_func op_3118;
extern cpuop_func op_3120;
extern cpuop_func op_3128;
extern cpuop_func op_3130;
extern cpuop_func op_3138;
extern cpuop_func op_3139;
extern cpuop_func op_313a;
extern cpuop_func op_313b;
extern cpuop_func op_313c;
extern cpuop_func op_3140;
extern cpuop_func op_3148;
extern cpuop_func op_3150;
extern cpuop_func op_3158;
extern cpuop_func op_3160;
extern cpuop_func op_3168;
extern cpuop_func op_3170;
extern cpuop_func op_3178;
extern cpuop_func op_3179;
extern cpuop_func op_317a;
extern cpuop_func op_317b;
extern cpuop_func op_317c;
extern cpuop_func op_3180;
extern cpuop_func op_3188;
extern cpuop_func op_3190;
extern cpuop_func op_3198;
extern cpuop_func op_31a0;
extern cpuop_func op_31a8;
extern cpuop_func op_31b0;
extern cpuop_func op_31b8;
extern cpuop_func op_31b9;
extern cpuop_func op_31ba;
extern cpuop_func op_31bb;
extern cpuop_func op_31bc;
extern cpuop_func op_31c0;
extern cpuop_func op_31c8;
extern cpuop_func op_31d0;
extern cpuop_func op_31d8;
extern cpuop_func op_31e0;
extern cpuop_func op_31e8;
extern cpuop_func op_31f0;
extern cpuop_func op_31f8;
extern cpuop_func op_31f9;
extern cpuop_func op_31fa;
extern cpuop_func op_31fb;
extern cpuop_func op_31fc;
extern cpuop_func op_33c0;
extern cpuop_func op_33c8;
extern cpuop_func op_33d0;
extern cpuop_func op_33d8;
extern cpuop_func op_33e0;
extern cpuop_func op_33e8;
extern cpuop_func op_33f0;
extern cpuop_func op_33f8;
extern cpuop_func op_33f9;
extern cpuop_func op_33fa;
extern cpuop_func op_33fb;
extern cpuop_func op_33fc;
extern cpuop_func op_4000;
extern cpuop_func op_4010;
extern cpuop_func op_4018;
extern cpuop_func op_4020;
extern cpuop_func op_4028;
extern cpuop_func op_4030;
extern cpuop_func op_4038;
extern cpuop_func op_4039;
extern cpuop_func op_4040;
extern cpuop_func op_4050;
extern cpuop_func op_4058;
extern cpuop_func op_4060;
extern cpuop_func op_4068;
extern cpuop_func op_4070;
extern cpuop_func op_4078;
extern cpuop_func op_4079;
extern cpuop_func op_4080;
extern cpuop_func op_4090;
extern cpuop_func op_4098;
extern cpuop_func op_40a0;
extern cpuop_func op_40a8;
extern cpuop_func op_40b0;
extern cpuop_func op_40b8;
extern cpuop_func op_40b9;
extern cpuop_func op_40c0;
extern cpuop_func op_40d0;
extern cpuop_func op_40d8;
extern cpuop_func op_40e0;
extern cpuop_func op_40e8;
extern cpuop_func op_40f0;
extern cpuop_func op_40f8;
extern cpuop_func op_40f9;
extern cpuop_func op_4100;
extern cpuop_func op_4110;
extern cpuop_func op_4118;
extern cpuop_func op_4120;
extern cpuop_func op_4128;
extern cpuop_func op_4130;
extern cpuop_func op_4138;
extern cpuop_func op_4139;
extern cpuop_func op_413a;
extern cpuop_func op_413b;
extern cpuop_func op_413c;
extern cpuop_func op_4180;
extern cpuop_func op_4190;
extern cpuop_func op_4198;
extern cpuop_func op_41a0;
extern cpuop_func op_41a8;
extern cpuop_func op_41b0;
extern cpuop_func op_41b8;
extern cpuop_func op_41b9;
extern cpuop_func op_41ba;
extern cpuop_func op_41bb;
extern cpuop_func op_41bc;
extern cpuop_func op_41d0;
extern cpuop_func op_41e8;
extern cpuop_func op_41f0;
extern cpuop_func op_41f8;
extern cpuop_func op_41f9;
extern cpuop_func op_41fa;
extern cpuop_func op_41fb;
extern cpuop_func op_4200;
extern cpuop_func op_4210;
extern cpuop_func op_4218;
extern cpuop_func op_4220;
extern cpuop_func op_4228;
extern cpuop_func op_4230;
extern cpuop_func op_4238;
extern cpuop_func op_4239;
extern cpuop_func op_4240;
extern cpuop_func op_4250;
extern cpuop_func op_4258;
extern cpuop_func op_4260;
extern cpuop_func op_4268;
extern cpuop_func op_4270;
extern cpuop_func op_4278;
extern cpuop_func op_4279;
extern cpuop_func op_4280;
extern cpuop_func op_4290;
extern cpuop_func op_4298;
extern cpuop_func op_42a0;
extern cpuop_func op_42a8;
extern cpuop_func op_42b0;
extern cpuop_func op_42b8;
extern cpuop_func op_42b9;
extern cpuop_func op_4400;
extern cpuop_func op_4410;
extern cpuop_func op_4418;
extern cpuop_func op_4420;
extern cpuop_func op_4428;
extern cpuop_func op_4430;
extern cpuop_func op_4438;
extern cpuop_func op_4439;
extern cpuop_func op_4440;
extern cpuop_func op_4450;
extern cpuop_func op_4458;
extern cpuop_func op_4460;
extern cpuop_func op_4468;
extern cpuop_func op_4470;
extern cpuop_func op_4478;
extern cpuop_func op_4479;
extern cpuop_func op_4480;
extern cpuop_func op_4490;
extern cpuop_func op_4498;
extern cpuop_func op_44a0;
extern cpuop_func op_44a8;
extern cpuop_func op_44b0;
extern cpuop_func op_44b8;
extern cpuop_func op_44b9;
extern cpuop_func op_44c0;
extern cpuop_func op_44d0;
extern cpuop_func op_44d8;
extern cpuop_func op_44e0;
extern cpuop_func op_44e8;
extern cpuop_func op_44f0;
extern cpuop_func op_44f8;
extern cpuop_func op_44f9;
extern cpuop_func op_44fa;
extern cpuop_func op_44fb;
extern cpuop_func op_44fc;
extern cpuop_func op_4600;
extern cpuop_func op_4610;
extern cpuop_func op_4618;
extern cpuop_func op_4620;
extern cpuop_func op_4628;
extern cpuop_func op_4630;
extern cpuop_func op_4638;
extern cpuop_func op_4639;
extern cpuop_func op_4640;
extern cpuop_func op_4650;
extern cpuop_func op_4658;
extern cpuop_func op_4660;
extern cpuop_func op_4668;
extern cpuop_func op_4670;
extern cpuop_func op_4678;
extern cpuop_func op_4679;
extern cpuop_func op_4680;
extern cpuop_func op_4690;
extern cpuop_func op_4698;
extern cpuop_func op_46a0;
extern cpuop_func op_46a8;
extern cpuop_func op_46b0;
extern cpuop_func op_46b8;
extern cpuop_func op_46b9;
extern cpuop_func op_46c0;
extern cpuop_func op_46d0;
extern cpuop_func op_46d8;
extern cpuop_func op_46e0;
extern cpuop_func op_46e8;
extern cpuop_func op_46f0;
extern cpuop_func op_46f8;
extern cpuop_func op_46f9;
extern cpuop_func op_46fa;
extern cpuop_func op_46fb;
extern cpuop_func op_46fc;
extern cpuop_func op_4800;
extern cpuop_func op_4810;
extern cpuop_func op_4818;
extern cpuop_func op_4820;
extern cpuop_func op_4828;
extern cpuop_func op_4830;
extern cpuop_func op_4838;
extern cpuop_func op_4839;
extern cpuop_func op_4840;
extern cpuop_func op_4850;
extern cpuop_func op_4868;
extern cpuop_func op_4870;
extern cpuop_func op_4878;
extern cpuop_func op_4879;
extern cpuop_func op_487a;
extern cpuop_func op_487b;
extern cpuop_func op_4880;
extern cpuop_func op_4890;
extern cpuop_func op_48a0;
extern cpuop_func op_48a8;
extern cpuop_func op_48b0;
extern cpuop_func op_48b8;
extern cpuop_func op_48b9;
extern cpuop_func op_48c0;
extern cpuop_func op_48d0;
extern cpuop_func op_48e0;
extern cpuop_func op_48e8;
extern cpuop_func op_48f0;
extern cpuop_func op_48f8;
extern cpuop_func op_48f9;
extern cpuop_func op_49c0;
extern cpuop_func op_4a00;
extern cpuop_func op_4a10;
extern cpuop_func op_4a18;
extern cpuop_func op_4a20;
extern cpuop_func op_4a28;
extern cpuop_func op_4a30;
extern cpuop_func op_4a38;
extern cpuop_func op_4a39;
extern cpuop_func op_4a3a;
extern cpuop_func op_4a3b;
extern cpuop_func op_4a3c;
extern cpuop_func op_4a40;
extern cpuop_func op_4a48;
extern cpuop_func op_4a50;
extern cpuop_func op_4a58;
extern cpuop_func op_4a60;
extern cpuop_func op_4a68;
extern cpuop_func op_4a70;
extern cpuop_func op_4a78;
extern cpuop_func op_4a79;
extern cpuop_func op_4a7a;
extern cpuop_func op_4a7b;
extern cpuop_func op_4a7c;
extern cpuop_func op_4a80;
extern cpuop_func op_4a88;
extern cpuop_func op_4a90;
extern cpuop_func op_4a98;
extern cpuop_func op_4aa0;
extern cpuop_func op_4aa8;
extern cpuop_func op_4ab0;
extern cpuop_func op_4ab8;
extern cpuop_func op_4ab9;
extern cpuop_func op_4aba;
extern cpuop_func op_4abb;
extern cpuop_func op_4abc;
extern cpuop_func op_4ac0;
extern cpuop_func op_4ad0;
extern cpuop_func op_4ad8;
extern cpuop_func op_4ae0;
extern cpuop_func op_4ae8;
extern cpuop_func op_4af0;
extern cpuop_func op_4af8;
extern cpuop_func op_4af9;
extern cpuop_func op_4c90;
extern cpuop_func op_4c98;
extern cpuop_func op_4ca8;
extern cpuop_func op_4cb0;
extern cpuop_func op_4cb8;
extern cpuop_func op_4cb9;
extern cpuop_func op_4cba;
extern cpuop_func op_4cbb;
extern cpuop_func op_4cd0;
extern cpuop_func op_4cd8;
extern cpuop_func op_4ce8;
extern cpuop_func op_4cf0;
extern cpuop_func op_4cf8;
extern cpuop_func op_4cf9;
extern cpuop_func op_4cfa;
extern cpuop_func op_4cfb;
extern cpuop_func op_4e40;
extern cpuop_func op_4e50;
extern cpuop_func op_4e58;
extern cpuop_func op_4e60;
extern cpuop_func op_4e68;
extern cpuop_func op_4e70;
extern cpuop_func op_4e71;
extern cpuop_func op_4e72;
extern cpuop_func op_4e73;
extern cpuop_func op_4e74;
extern cpuop_func op_4e75;
extern cpuop_func op_4e76;
extern cpuop_func op_4e77;
extern cpuop_func op_4e90;
extern cpuop_func op_4ea8;
extern cpuop_func op_4eb0;
extern cpuop_func op_4eb8;
extern cpuop_func op_4eb9;
extern cpuop_func op_4eba;
extern cpuop_func op_4ebb;
extern cpuop_func op_4ed0;
extern cpuop_func op_4ee8;
extern cpuop_func op_4ef0;
extern cpuop_func op_4ef8;
extern cpuop_func op_4ef9;
extern cpuop_func op_4efa;
extern cpuop_func op_4efb;
extern cpuop_func op_5000;
extern cpuop_func op_5010;
extern cpuop_func op_5018;
extern cpuop_func op_5020;
extern cpuop_func op_5028;
extern cpuop_func op_5030;
extern cpuop_func op_5038;
extern cpuop_func op_5039;
extern cpuop_func op_5040;
extern cpuop_func op_5048;
extern cpuop_func op_5050;
extern cpuop_func op_5058;
extern cpuop_func op_5060;
extern cpuop_func op_5068;
extern cpuop_func op_5070;
extern cpuop_func op_5078;
extern cpuop_func op_5079;
extern cpuop_func op_5080;
extern cpuop_func op_5088;
extern cpuop_func op_5090;
extern cpuop_func op_5098;
extern cpuop_func op_50a0;
extern cpuop_func op_50a8;
extern cpuop_func op_50b0;
extern cpuop_func op_50b8;
extern cpuop_func op_50b9;
extern cpuop_func op_50c0;
extern cpuop_func op_50c8;
extern cpuop_func op_50d0;
extern cpuop_func op_50d8;
extern cpuop_func op_50e0;
extern cpuop_func op_50e8;
extern cpuop_func op_50f0;
extern cpuop_func op_50f8;
extern cpuop_func op_50f9;
extern cpuop_func op_5100;
extern cpuop_func op_5110;
extern cpuop_func op_5118;
extern cpuop_func op_5120;
extern cpuop_func op_5128;
extern cpuop_func op_5130;
extern cpuop_func op_5138;
extern cpuop_func op_5139;
extern cpuop_func op_5140;
extern cpuop_func op_5148;
extern cpuop_func op_5150;
extern cpuop_func op_5158;
extern cpuop_func op_5160;
extern cpuop_func op_5168;
extern cpuop_func op_5170;
extern cpuop_func op_5178;
extern cpuop_func op_5179;
extern cpuop_func op_5180;
extern cpuop_func op_5188;
extern cpuop_func op_5190;
extern cpuop_func op_5198;
extern cpuop_func op_51a0;
extern cpuop_func op_51a8;
extern cpuop_func op_51b0;
extern cpuop_func op_51b8;
extern cpuop_func op_51b9;
extern cpuop_func op_51c0;
extern cpuop_func op_51c8;
extern cpuop_func op_51d0;
extern cpuop_func op_51d8;
extern cpuop_func op_51e0;
extern cpuop_func op_51e8;
extern cpuop_func op_51f0;
extern cpuop_func op_51f8;
extern cpuop_func op_51f9;
extern cpuop_func op_52c0;
extern cpuop_func op_52c8;
extern cpuop_func op_52d0;
extern cpuop_func op_52d8;
extern cpuop_func op_52e0;
extern cpuop_func op_52e8;
extern cpuop_func op_52f0;
extern cpuop_func op_52f8;
extern cpuop_func op_52f9;
extern cpuop_func op_53c0;
extern cpuop_func op_53c8;
extern cpuop_func op_53d0;
extern cpuop_func op_53d8;
extern cpuop_func op_53e0;
extern cpuop_func op_53e8;
extern cpuop_func op_53f0;
extern cpuop_func op_53f8;
extern cpuop_func op_53f9;
extern cpuop_func op_54c0;
extern cpuop_func op_54c8;
extern cpuop_func op_54d0;
extern cpuop_func op_54d8;
extern cpuop_func op_54e0;
extern cpuop_func op_54e8;
extern cpuop_func op_54f0;
extern cpuop_func op_54f8;
extern cpuop_func op_54f9;
extern cpuop_func op_55c0;
extern cpuop_func op_55c8;
extern cpuop_func op_55d0;
extern cpuop_func op_55d8;
extern cpuop_func op_55e0;
extern cpuop_func op_55e8;
extern cpuop_func op_55f0;
extern cpuop_func op_55f8;
extern cpuop_func op_55f9;
extern cpuop_func op_56c0;
extern cpuop_func op_56c8;
extern cpuop_func op_56d0;
extern cpuop_func op_56d8;
extern cpuop_func op_56e0;
extern cpuop_func op_56e8;
extern cpuop_func op_56f0;
extern cpuop_func op_56f8;
extern cpuop_func op_56f9;
extern cpuop_func op_57c0;
extern cpuop_func op_57c8;
extern cpuop_func op_57d0;
extern cpuop_func op_57d8;
extern cpuop_func op_57e0;
extern cpuop_func op_57e8;
extern cpuop_func op_57f0;
extern cpuop_func op_57f8;
extern cpuop_func op_57f9;
extern cpuop_func op_58c0;
extern cpuop_func op_58c8;
extern cpuop_func op_58d0;
extern cpuop_func op_58d8;
extern cpuop_func op_58e0;
extern cpuop_func op_58e8;
extern cpuop_func op_58f0;
extern cpuop_func op_58f8;
extern cpuop_func op_58f9;
extern cpuop_func op_59c0;
extern cpuop_func op_59c8;
extern cpuop_func op_59d0;
extern cpuop_func op_59d8;
extern cpuop_func op_59e0;
extern cpuop_func op_59e8;
extern cpuop_func op_59f0;
extern cpuop_func op_59f8;
extern cpuop_func op_59f9;
extern cpuop_func op_5ac0;
extern cpuop_func op_5ac8;
extern cpuop_func op_5ad0;
extern cpuop_func op_5ad8;
extern cpuop_func op_5ae0;
extern cpuop_func op_5ae8;
extern cpuop_func op_5af0;
extern cpuop_func op_5af8;
extern cpuop_func op_5af9;
extern cpuop_func op_5bc0;
extern cpuop_func op_5bc8;
extern cpuop_func op_5bd0;
extern cpuop_func op_5bd8;
extern cpuop_func op_5be0;
extern cpuop_func op_5be8;
extern cpuop_func op_5bf0;
extern cpuop_func op_5bf8;
extern cpuop_func op_5bf9;
extern cpuop_func op_5cc0;
extern cpuop_func op_5cc8;
extern cpuop_func op_5cd0;
extern cpuop_func op_5cd8;
extern cpuop_func op_5ce0;
extern cpuop_func op_5ce8;
extern cpuop_func op_5cf0;
extern cpuop_func op_5cf8;
extern cpuop_func op_5cf9;
extern cpuop_func op_5dc0;
extern cpuop_func op_5dc8;
extern cpuop_func op_5dd0;
extern cpuop_func op_5dd8;
extern cpuop_func op_5de0;
extern cpuop_func op_5de8;
extern cpuop_func op_5df0;
extern cpuop_func op_5df8;
extern cpuop_func op_5df9;
extern cpuop_func op_5ec0;
extern cpuop_func op_5ec8;
extern cpuop_func op_5ed0;
extern cpuop_func op_5ed8;
extern cpuop_func op_5ee0;
extern cpuop_func op_5ee8;
extern cpuop_func op_5ef0;
extern cpuop_func op_5ef8;
extern cpuop_func op_5ef9;
extern cpuop_func op_5fc0;
extern cpuop_func op_5fc8;
extern cpuop_func op_5fd0;
extern cpuop_func op_5fd8;
extern cpuop_func op_5fe0;
extern cpuop_func op_5fe8;
extern cpuop_func op_5ff0;
extern cpuop_func op_5ff8;
extern cpuop_func op_5ff9;
extern cpuop_func op_6000;
extern cpuop_func op_6001;
extern cpuop_func op_60ff;
extern cpuop_func op_6100;
extern cpuop_func op_6101;
extern cpuop_func op_61ff;
extern cpuop_func op_6200;
extern cpuop_func op_6201;
extern cpuop_func op_62ff;
extern cpuop_func op_6300;
extern cpuop_func op_6301;
extern cpuop_func op_63ff;
extern cpuop_func op_6400;
extern cpuop_func op_6401;
extern cpuop_func op_64ff;
extern cpuop_func op_6500;
extern cpuop_func op_6501;
extern cpuop_func op_65ff;
extern cpuop_func op_6600;
extern cpuop_func op_6601;
extern cpuop_func op_66ff;
extern cpuop_func op_6700;
extern cpuop_func op_6701;
extern cpuop_func op_67ff;
extern cpuop_func op_6800;
extern cpuop_func op_6801;
extern cpuop_func op_68ff;
extern cpuop_func op_6900;
extern cpuop_func op_6901;
extern cpuop_func op_69ff;
extern cpuop_func op_6a00;
extern cpuop_func op_6a01;
extern cpuop_func op_6aff;
extern cpuop_func op_6b00;
extern cpuop_func op_6b01;
extern cpuop_func op_6bff;
extern cpuop_func op_6c00;
extern cpuop_func op_6c01;
extern cpuop_func op_6cff;
extern cpuop_func op_6d00;
extern cpuop_func op_6d01;
extern cpuop_func op_6dff;
extern cpuop_func op_6e00;
extern cpuop_func op_6e01;
extern cpuop_func op_6eff;
extern cpuop_func op_6f00;
extern cpuop_func op_6f01;
extern cpuop_func op_6fff;
extern cpuop_func op_7000;
extern cpuop_func op_8000;
extern cpuop_func op_8010;
extern cpuop_func op_8018;
extern cpuop_func op_8020;
extern cpuop_func op_8028;
extern cpuop_func op_8030;
extern cpuop_func op_8038;
extern cpuop_func op_8039;
extern cpuop_func op_803a;
extern cpuop_func op_803b;
extern cpuop_func op_803c;
extern cpuop_func op_8040;
extern cpuop_func op_8050;
extern cpuop_func op_8058;
extern cpuop_func op_8060;
extern cpuop_func op_8068;
extern cpuop_func op_8070;
extern cpuop_func op_8078;
extern cpuop_func op_8079;
extern cpuop_func op_807a;
extern cpuop_func op_807b;
extern cpuop_func op_807c;
extern cpuop_func op_8080;
extern cpuop_func op_8090;
extern cpuop_func op_8098;
extern cpuop_func op_80a0;
extern cpuop_func op_80a8;
extern cpuop_func op_80b0;
extern cpuop_func op_80b8;
extern cpuop_func op_80b9;
extern cpuop_func op_80ba;
extern cpuop_func op_80bb;
extern cpuop_func op_80bc;
extern cpuop_func op_80c0;
extern cpuop_func op_80d0;
extern cpuop_func op_80d8;
extern cpuop_func op_80e0;
extern cpuop_func op_80e8;
extern cpuop_func op_80f0;
extern cpuop_func op_80f8;
extern cpuop_func op_80f9;
extern cpuop_func op_80fa;
extern cpuop_func op_80fb;
extern cpuop_func op_80fc;
extern cpuop_func op_8100;
extern cpuop_func op_8108;
extern cpuop_func op_8110;
extern cpuop_func op_8118;
extern cpuop_func op_8120;
extern cpuop_func op_8128;
extern cpuop_func op_8130;
extern cpuop_func op_8138;
extern cpuop_func op_8139;
extern cpuop_func op_8150;
extern cpuop_func op_8158;
extern cpuop_func op_8160;
extern cpuop_func op_8168;
extern cpuop_func op_8170;
extern cpuop_func op_8178;
extern cpuop_func op_8179;
extern cpuop_func op_8190;
extern cpuop_func op_8198;
extern cpuop_func op_81a0;
extern cpuop_func op_81a8;
extern cpuop_func op_81b0;
extern cpuop_func op_81b8;
extern cpuop_func op_81b9;
extern cpuop_func op_81c0;
extern cpuop_func op_81d0;
extern cpuop_func op_81d8;
extern cpuop_func op_81e0;
extern cpuop_func op_81e8;
extern cpuop_func op_81f0;
extern cpuop_func op_81f8;
extern cpuop_func op_81f9;
extern cpuop_func op_81fa;
extern cpuop_func op_81fb;
extern cpuop_func op_81fc;
extern cpuop_func op_9000;
extern cpuop_func op_9010;
extern cpuop_func op_9018;
extern cpuop_func op_9020;
extern cpuop_func op_9028;
extern cpuop_func op_9030;
extern cpuop_func op_9038;
extern cpuop_func op_9039;
extern cpuop_func op_903a;
extern cpuop_func op_903b;
extern cpuop_func op_903c;
extern cpuop_func op_9040;
extern cpuop_func op_9048;
extern cpuop_func op_9050;
extern cpuop_func op_9058;
extern cpuop_func op_9060;
extern cpuop_func op_9068;
extern cpuop_func op_9070;
extern cpuop_func op_9078;
extern cpuop_func op_9079;
extern cpuop_func op_907a;
extern cpuop_func op_907b;
extern cpuop_func op_907c;
extern cpuop_func op_9080;
extern cpuop_func op_9088;
extern cpuop_func op_9090;
extern cpuop_func op_9098;
extern cpuop_func op_90a0;
extern cpuop_func op_90a8;
extern cpuop_func op_90b0;
extern cpuop_func op_90b8;
extern cpuop_func op_90b9;
extern cpuop_func op_90ba;
extern cpuop_func op_90bb;
extern cpuop_func op_90bc;
extern cpuop_func op_90c0;
extern cpuop_func op_90c8;
extern cpuop_func op_90d0;
extern cpuop_func op_90d8;
extern cpuop_func op_90e0;
extern cpuop_func op_90e8;
extern cpuop_func op_90f0;
extern cpuop_func op_90f8;
extern cpuop_func op_90f9;
extern cpuop_func op_90fa;
extern cpuop_func op_90fb;
extern cpuop_func op_90fc;
extern cpuop_func op_9100;
extern cpuop_func op_9108;
extern cpuop_func op_9110;
extern cpuop_func op_9118;
extern cpuop_func op_9120;
extern cpuop_func op_9128;
extern cpuop_func op_9130;
extern cpuop_func op_9138;
extern cpuop_func op_9139;
extern cpuop_func op_9140;
extern cpuop_func op_9148;
extern cpuop_func op_9150;
extern cpuop_func op_9158;
extern cpuop_func op_9160;
extern cpuop_func op_9168;
extern cpuop_func op_9170;
extern cpuop_func op_9178;
extern cpuop_func op_9179;
extern cpuop_func op_9180;
extern cpuop_func op_9188;
extern cpuop_func op_9190;
extern cpuop_func op_9198;
extern cpuop_func op_91a0;
extern cpuop_func op_91a8;
extern cpuop_func op_91b0;
extern cpuop_func op_91b8;
extern cpuop_func op_91b9;
extern cpuop_func op_91c0;
extern cpuop_func op_91c8;
extern cpuop_func op_91d0;
extern cpuop_func op_91d8;
extern cpuop_func op_91e0;
extern cpuop_func op_91e8;
extern cpuop_func op_91f0;
extern cpuop_func op_91f8;
extern cpuop_func op_91f9;
extern cpuop_func op_91fa;
extern cpuop_func op_91fb;
extern cpuop_func op_91fc;
extern cpuop_func op_b000;
extern cpuop_func op_b010;
extern cpuop_func op_b018;
extern cpuop_func op_b020;
extern cpuop_func op_b028;
extern cpuop_func op_b030;
extern cpuop_func op_b038;
extern cpuop_func op_b039;
extern cpuop_func op_b03a;
extern cpuop_func op_b03b;
extern cpuop_func op_b03c;
extern cpuop_func op_b040;
extern cpuop_func op_b048;
extern cpuop_func op_b050;
extern cpuop_func op_b058;
extern cpuop_func op_b060;
extern cpuop_func op_b068;
extern cpuop_func op_b070;
extern cpuop_func op_b078;
extern cpuop_func op_b079;
extern cpuop_func op_b07a;
extern cpuop_func op_b07b;
extern cpuop_func op_b07c;
extern cpuop_func op_b080;
extern cpuop_func op_b088;
extern cpuop_func op_b090;
extern cpuop_func op_b098;
extern cpuop_func op_b0a0;
extern cpuop_func op_b0a8;
extern cpuop_func op_b0b0;
extern cpuop_func op_b0b8;
extern cpuop_func op_b0b9;
extern cpuop_func op_b0ba;
extern cpuop_func op_b0bb;
extern cpuop_func op_b0bc;
extern cpuop_func op_b0c0;
extern cpuop_func op_b0c8;
extern cpuop_func op_b0d0;
extern cpuop_func op_b0d8;
extern cpuop_func op_b0e0;
extern cpuop_func op_b0e8;
extern cpuop_func op_b0f0;
extern cpuop_func op_b0f8;
extern cpuop_func op_b0f9;
extern cpuop_func op_b0fa;
extern cpuop_func op_b0fb;
extern cpuop_func op_b0fc;
extern cpuop_func op_b100;
extern cpuop_func op_b108;
extern cpuop_func op_b110;
extern cpuop_func op_b118;
extern cpuop_func op_b120;
extern cpuop_func op_b128;
extern cpuop_func op_b130;
extern cpuop_func op_b138;
extern cpuop_func op_b139;
extern cpuop_func op_b140;
extern cpuop_func op_b148;
extern cpuop_func op_b150;
extern cpuop_func op_b158;
extern cpuop_func op_b160;
extern cpuop_func op_b168;
extern cpuop_func op_b170;
extern cpuop_func op_b178;
extern cpuop_func op_b179;
extern cpuop_func op_b180;
extern cpuop_func op_b188;
extern cpuop_func op_b190;
extern cpuop_func op_b198;
extern cpuop_func op_b1a0;
extern cpuop_func op_b1a8;
extern cpuop_func op_b1b0;
extern cpuop_func op_b1b8;
extern cpuop_func op_b1b9;
extern cpuop_func op_b1c0;
extern cpuop_func op_b1c8;
extern cpuop_func op_b1d0;
extern cpuop_func op_b1d8;
extern cpuop_func op_b1e0;
extern cpuop_func op_b1e8;
extern cpuop_func op_b1f0;
extern cpuop_func op_b1f8;
extern cpuop_func op_b1f9;
extern cpuop_func op_b1fa;
extern cpuop_func op_b1fb;
extern cpuop_func op_b1fc;
extern cpuop_func op_c000;
extern cpuop_func op_c010;
extern cpuop_func op_c018;
extern cpuop_func op_c020;
extern cpuop_func op_c028;
extern cpuop_func op_c030;
extern cpuop_func op_c038;
extern cpuop_func op_c039;
extern cpuop_func op_c03a;
extern cpuop_func op_c03b;
extern cpuop_func op_c03c;
extern cpuop_func op_c040;
extern cpuop_func op_c050;
extern cpuop_func op_c058;
extern cpuop_func op_c060;
extern cpuop_func op_c068;
extern cpuop_func op_c070;
extern cpuop_func op_c078;
extern cpuop_func op_c079;
extern cpuop_func op_c07a;
extern cpuop_func op_c07b;
extern cpuop_func op_c07c;
extern cpuop_func op_c080;
extern cpuop_func op_c090;
extern cpuop_func op_c098;
extern cpuop_func op_c0a0;
extern cpuop_func op_c0a8;
extern cpuop_func op_c0b0;
extern cpuop_func op_c0b8;
extern cpuop_func op_c0b9;
extern cpuop_func op_c0ba;
extern cpuop_func op_c0bb;
extern cpuop_func op_c0bc;
extern cpuop_func op_c0c0;
extern cpuop_func op_c0d0;
extern cpuop_func op_c0d8;
extern cpuop_func op_c0e0;
extern cpuop_func op_c0e8;
extern cpuop_func op_c0f0;
extern cpuop_func op_c0f8;
extern cpuop_func op_c0f9;
extern cpuop_func op_c0fa;
extern cpuop_func op_c0fb;
extern cpuop_func op_c0fc;
extern cpuop_func op_c100;
extern cpuop_func op_c108;
extern cpuop_func op_c110;
extern cpuop_func op_c118;
extern cpuop_func op_c120;
extern cpuop_func op_c128;
extern cpuop_func op_c130;
extern cpuop_func op_c138;
extern cpuop_func op_c139;
extern cpuop_func op_c140;
extern cpuop_func op_c148;
extern cpuop_func op_c150;
extern cpuop_func op_c158;
extern cpuop_func op_c160;
extern cpuop_func op_c168;
extern cpuop_func op_c170;
extern cpuop_func op_c178;
extern cpuop_func op_c179;
extern cpuop_func op_c188;
extern cpuop_func op_c190;
extern cpuop_func op_c198;
extern cpuop_func op_c1a0;
extern cpuop_func op_c1a8;
extern cpuop_func op_c1b0;
extern cpuop_func op_c1b8;
extern cpuop_func op_c1b9;
extern cpuop_func op_c1c0;
extern cpuop_func op_c1d0;
extern cpuop_func op_c1d8;
extern cpuop_func op_c1e0;
extern cpuop_func op_c1e8;
extern cpuop_func op_c1f0;
extern cpuop_func op_c1f8;
extern cpuop_func op_c1f9;
extern cpuop_func op_c1fa;
extern cpuop_func op_c1fb;
extern cpuop_func op_c1fc;
extern cpuop_func op_d000;
extern cpuop_func op_d010;
extern cpuop_func op_d018;
extern cpuop_func op_d020;
extern cpuop_func op_d028;
extern cpuop_func op_d030;
extern cpuop_func op_d038;
extern cpuop_func op_d039;
extern cpuop_func op_d03a;
extern cpuop_func op_d03b;
extern cpuop_func op_d03c;
extern cpuop_func op_d040;
extern cpuop_func op_d048;
extern cpuop_func op_d050;
extern cpuop_func op_d058;
extern cpuop_func op_d060;
extern cpuop_func op_d068;
extern cpuop_func op_d070;
extern cpuop_func op_d078;
extern cpuop_func op_d079;
extern cpuop_func op_d07a;
extern cpuop_func op_d07b;
extern cpuop_func op_d07c;
extern cpuop_func op_d080;
extern cpuop_func op_d088;
extern cpuop_func op_d090;
extern cpuop_func op_d098;
extern cpuop_func op_d0a0;
extern cpuop_func op_d0a8;
extern cpuop_func op_d0b0;
extern cpuop_func op_d0b8;
extern cpuop_func op_d0b9;
extern cpuop_func op_d0ba;
extern cpuop_func op_d0bb;
extern cpuop_func op_d0bc;
extern cpuop_func op_d0c0;
extern cpuop_func op_d0c8;
extern cpuop_func op_d0d0;
extern cpuop_func op_d0d8;
extern cpuop_func op_d0e0;
extern cpuop_func op_d0e8;
extern cpuop_func op_d0f0;
extern cpuop_func op_d0f8;
extern cpuop_func op_d0f9;
extern cpuop_func op_d0fa;
extern cpuop_func op_d0fb;
extern cpuop_func op_d0fc;
extern cpuop_func op_d100;
extern cpuop_func op_d108;
extern cpuop_func op_d110;
extern cpuop_func op_d118;
extern cpuop_func op_d120;
extern cpuop_func op_d128;
extern cpuop_func op_d130;
extern cpuop_func op_d138;
extern cpuop_func op_d139;
extern cpuop_func op_d140;
extern cpuop_func op_d148;
extern cpuop_func op_d150;
extern cpuop_func op_d158;
extern cpuop_func op_d160;
extern cpuop_func op_d168;
extern cpuop_func op_d170;
extern cpuop_func op_d178;
extern cpuop_func op_d179;
extern cpuop_func op_d180;
extern cpuop_func op_d188;
extern cpuop_func op_d190;
extern cpuop_func op_d198;
extern cpuop_func op_d1a0;
extern cpuop_func op_d1a8;
extern cpuop_func op_d1b0;
extern cpuop_func op_d1b8;
extern cpuop_func op_d1b9;
extern cpuop_func op_d1c0;
extern cpuop_func op_d1c8;
extern cpuop_func op_d1d0;
extern cpuop_func op_d1d8;
extern cpuop_func op_d1e0;
extern cpuop_func op_d1e8;
extern cpuop_func op_d1f0;
extern cpuop_func op_d1f8;
extern cpuop_func op_d1f9;
extern cpuop_func op_d1fa;
extern cpuop_func op_d1fb;
extern cpuop_func op_d1fc;
extern cpuop_func op_e000;
extern cpuop_func op_e008;
extern cpuop_func op_e010;
extern cpuop_func op_e018;
extern cpuop_func op_e020;
extern cpuop_func op_e028;
extern cpuop_func op_e030;
extern cpuop_func op_e038;
extern cpuop_func op_e040;
extern cpuop_func op_e048;
extern cpuop_func op_e050;
extern cpuop_func op_e058;
extern cpuop_func op_e060;
extern cpuop_func op_e068;
extern cpuop_func op_e070;
extern cpuop_func op_e078;
extern cpuop_func op_e080;
extern cpuop_func op_e088;
extern cpuop_func op_e090;
extern cpuop_func op_e098;
extern cpuop_func op_e0a0;
extern cpuop_func op_e0a8;
extern cpuop_func op_e0b0;
extern cpuop_func op_e0b8;
extern cpuop_func op_e0d0;
extern cpuop_func op_e0d8;
extern cpuop_func op_e0e0;
extern cpuop_func op_e0e8;
extern cpuop_func op_e0f0;
extern cpuop_func op_e0f8;
extern cpuop_func op_e0f9;
extern cpuop_func op_e100;
extern cpuop_func op_e108;
extern cpuop_func op_e110;
extern cpuop_func op_e118;
extern cpuop_func op_e120;
extern cpuop_func op_e128;
extern cpuop_func op_e130;
extern cpuop_func op_e138;
extern cpuop_func op_e140;
extern cpuop_func op_e148;
extern cpuop_func op_e150;
extern cpuop_func op_e158;
extern cpuop_func op_e160;
extern cpuop_func op_e168;
extern cpuop_func op_e170;
extern cpuop_func op_e178;
extern cpuop_func op_e180;
extern cpuop_func op_e188;
extern cpuop_func op_e190;
extern cpuop_func op_e198;
extern cpuop_func op_e1a0;
extern cpuop_func op_e1a8;
extern cpuop_func op_e1b0;
extern cpuop_func op_e1b8;
extern cpuop_func op_e1d0;
extern cpuop_func op_e1d8;
extern cpuop_func op_e1e0;
extern cpuop_func op_e1e8;
extern cpuop_func op_e1f0;
extern cpuop_func op_e1f8;
extern cpuop_func op_e1f9;
extern cpuop_func op_e2d0;
extern cpuop_func op_e2d8;
extern cpuop_func op_e2e0;
extern cpuop_func op_e2e8;
extern cpuop_func op_e2f0;
extern cpuop_func op_e2f8;
extern cpuop_func op_e2f9;
extern cpuop_func op_e3d0;
extern cpuop_func op_e3d8;
extern cpuop_func op_e3e0;
extern cpuop_func op_e3e8;
extern cpuop_func op_e3f0;
extern cpuop_func op_e3f8;
extern cpuop_func op_e3f9;
extern cpuop_func op_e4d0;
extern cpuop_func op_e4d8;
extern cpuop_func op_e4e0;
extern cpuop_func op_e4e8;
extern cpuop_func op_e4f0;
extern cpuop_func op_e4f8;
extern cpuop_func op_e4f9;
extern cpuop_func op_e5d0;
extern cpuop_func op_e5d8;
extern cpuop_func op_e5e0;
extern cpuop_func op_e5e8;
extern cpuop_func op_e5f0;
extern cpuop_func op_e5f8;
extern cpuop_func op_e5f9;
extern cpuop_func op_e6d0;
extern cpuop_func op_e6d8;
extern cpuop_func op_e6e0;
extern cpuop_func op_e6e8;
extern cpuop_func op_e6f0;
extern cpuop_func op_e6f8;
extern cpuop_func op_e6f9;
extern cpuop_func op_e7d0;
extern cpuop_func op_e7d8;
extern cpuop_func op_e7e0;
extern cpuop_func op_e7e8;
extern cpuop_func op_e7f0;
extern cpuop_func op_e7f8;
extern cpuop_func op_e7f9;
#endif
#ifndef CYCLETBL_H
#define CYCLETBL_H

/********************************************************************/
/* cycletbl.h                                        *BETA RELEASE*	*/
/* Instruction timing table for the MC_MUSASHI000              Revision 0	*/
/*                                                                  */
/* ©1998 Brian Verre (wverre@execpc.com)                            */
/*                                                                  */
/* This file contains a timing table which provides accurate        */
/* cycle based timing for the UAE 68000 core. It was created        */
/* for the benefit of all emulators using the core.	This file       */
/* may be freely distributed with the core as long as it remains    */
/* unchanged. Please contact me if you have any suggestions         */
/*                                                                  */
/* Changes: updated opcode8.c, mc68kmem.c, and cpudefs.h files are 	*/
/* required for this table to function.	The DIVS/DIVU opcodes in    */
/* opcode8.c have been modified to decrement MC_MUSASHI000_ICount, and    */
/* MC_MUSASHI000_ICount has been extern'd in cpudefs.h                    */
/*                                                                  */
/* A few notes: this table is about 95% complete; MULS/MULU timing  */
/* is not yet accurate; a few other instructions were guesstimated. */
/*  E800-EFFF are not done yet. 99% of frequently called opcodes    */
/* are accurate, so the table should work fine until it is updated.	*/
/********************************************************************/

unsigned char cycletbl[65536] =
{
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32,
	34, 34, 34, 34, 34, 34, 34, 34, 32, 36, 32, 32, 32, 32, 32, 32,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32,
	34, 34, 34, 34, 34, 34, 34, 34, 32, 36, 32, 32, 32, 32, 32, 32,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32,
	34, 34, 34, 34, 34, 34, 34, 34, 32, 36, 32, 32, 32, 32, 32, 32,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32,
	34, 34, 34, 34, 34, 34, 34, 34, 32, 36, 32, 32, 32, 32, 32, 32,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 24, 24, 20, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 24, 24, 20, 20, 20, 20,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 24, 24, 20, 20, 20, 20,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32,
	34, 34, 34, 34, 34, 34, 34, 34, 32, 36, 32, 32, 32, 32, 32, 32,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 28, 28, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32,
	34, 34, 34, 34, 34, 34, 34, 34, 32, 36, 32, 32, 32, 32, 32, 32,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6, 16, 16, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 24, 24, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32,
	34, 34, 34, 34, 34, 34, 34, 34, 32, 36, 32, 34, 28, 32, 32, 32,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	28, 28, 28, 28, 28, 28, 28, 28, 30, 30, 30, 30, 30, 30, 30, 30,
	32, 32, 32, 32, 32, 32, 32, 32, 30, 34, 30, 32, 26, 30, 30, 30,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28,
	30, 30, 30, 30, 30, 30, 30, 30, 28, 32, 28, 30, 24, 28, 28, 28,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 26, 20, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 24, 18, 22, 22, 22,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	46, 46, 46, 46, 46, 46, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48,
	50, 50, 50, 50, 50, 50, 50, 50, 48, 52, 48, 50, 44, 48, 48, 48,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 14, 14, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 14, 14, 14, 14, 14,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	46, 46, 46, 46, 46, 46, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48,
	50, 50, 50, 50, 50, 50, 50, 50, 48, 52, 48, 50, 44, 48, 48, 48,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	46, 46, 46, 46, 46, 46, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48,
	50, 50, 50, 50, 50, 50, 50, 50, 48, 52, 48, 50, 44, 48, 48, 48,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 22, 16, 20, 20, 20,
	40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	46, 46, 46, 46, 46, 46, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48,
	50, 50, 50, 50, 50, 50, 50, 50, 48, 52, 48, 50, 44, 48, 48, 48,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 20, 24, 20, 20, 20, 20, 20, 20,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	20, 20, 20, 20, 20, 20, 20, 20, 16, 20, 16, 20, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 20, 20, 20, 20, 20, 20, 20, 20,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	46, 46, 46, 46, 46, 46, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48,
	50, 50, 50, 50, 50, 50, 50, 50, 48, 52, 48, 50, 44, 48, 48, 48,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 16, 16, 16, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 16, 16, 16, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 20, 20, 20, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 22, 22, 22, 22, 22, 22, 22, 22,
	24, 24, 24, 24, 24, 24, 24, 24, 22, 26, 22, 22, 22, 22, 22, 22,
	40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	46, 46, 46, 46, 46, 46, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48,
	50, 50, 50, 50, 50, 50, 50, 50, 48, 52, 48, 50, 44, 48, 48, 48,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	22, 22, 22, 22, 22, 22, 22, 22, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	46, 46, 46, 46, 46, 46, 46, 46, 48, 48, 48, 48, 48, 48, 48, 48,
	50, 50, 50, 50, 50, 50, 50, 50, 48, 52, 48, 50, 44, 48, 48, 48,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	34, 34, 34, 34, 34, 34, 34, 34, 12, 12, 12, 12, 12, 12, 12, 12,
	16, 16, 16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	132,  4,  4, 20, 20, 16, 16, 20,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	16, 16, 16, 16, 16, 16, 16, 16,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	22, 22, 22, 22, 22, 22, 22, 22, 18, 20, 18, 22, 18, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4, 10, 10, 10, 10, 10, 10, 10, 10,
	14, 14, 14, 14, 14, 14, 14, 14, 10, 12, 10, 14, 10, 10, 10, 10,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 12, 12, 12, 12, 12, 12, 12, 12,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  8, 12,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 14, 14, 14, 14, 14, 14, 14, 14,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 0,  0,  0,  0,  0,  0,  0,  0, 12, 12, 12, 12, 12, 12, 12, 12,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 14, 18, 14, 16, 10, 14, 14, 14,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 20, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 14, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 6,  6,  6,  6,  6,  6,  6,  6, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12,  6,  6,  6,  6,  6,  6,  6,  6,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	54, 54, 54, 54, 54, 54, 54, 54, 12, 12, 12, 12, 12, 12, 12, 12,
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	60, 60, 60, 60, 60, 60, 60, 60, 62, 62, 62, 62, 62, 62, 62, 62,
	64, 64, 64, 64, 64, 64, 64, 64, 62, 66, 62, 64, 58, 62, 62, 62,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	10, 10, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 12, 16, 12, 14,  8, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 18, 12, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 4,  4,  4,  4,  4,  4,  4,  4, 18, 18, 18, 18, 18, 18, 18, 18,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	 8,  8,  8,  8,  8,  8,  8,  8, 30, 30, 30, 30, 30, 30, 30, 30,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	22, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24, 24, 24, 24, 24,
	26, 26, 26, 26, 26, 26, 26, 26, 24, 28, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	16, 16, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18,
	20, 20, 20, 20, 20, 20, 20, 20, 18, 22, 18, 20, 16, 18, 18, 18,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	 6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	14, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16,
	18, 18, 18, 18, 18, 18, 18, 18, 16, 20, 16, 16, 16, 16, 16, 16,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34
};

const unsigned char exception_cycles[48] =
{
	0, 0, 0, 0, 38, 42, 44, 38, 38, 0, 38, 38, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 46, 46, 46, 46, 46, 46, 46, 46,
	38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38
};

#endif
#ifndef __MCreadcpu__
#define __MCreadcpu__

typedef enum {
  Dreg, Areg, Aind, Aipi, Apdi, Ad16, Ad8r,
  absw, absl, PC16, PC8r, imm, imm0, imm1, imm2, immi, am_unknown, am_illg
} amodes;

typedef enum {
    i_ILLG,

    i_OR, i_AND, i_EOR, i_ORSR, i_ANDSR, i_EORSR,
    i_SUB, i_SUBA, i_SUBX, i_SBCD,
    i_ADD, i_ADDA, i_ADDX, i_ABCD,
    i_NEG, i_NEGX, i_NBCD, i_CLR, i_NOT, i_TST,
    i_BTST, i_BCHG, i_BCLR, i_BSET,
    i_CMP, i_CMPM, i_CMPA,
    i_MVPRM, i_MVPMR, i_MOVE, i_MOVEA, i_MVSR2, i_MV2SR,
    i_SWAP, i_EXG, i_EXT, i_MVMEL, i_MVMLE,
    i_TRAP, i_MVR2USP, i_MVUSP2R, i_RESET, i_NOP, i_STOP, i_RTE, i_RTD,
    i_LINK, i_UNLK,
    i_RTS, i_TRAPV, i_RTR,
    i_JSR, i_JMP, i_BSR, i_Bcc,
    i_LEA, i_PEA, i_DBcc, i_Scc,
    i_DIVU, i_DIVS, i_MULU, i_MULS,
    i_ASR, i_ASL, i_LSR, i_LSL, i_ROL, i_ROR, i_ROXL, i_ROXR,
    i_ASRW, i_ASLW, i_LSRW, i_LSLW, i_ROLW, i_RORW, i_ROXLW, i_ROXRW,
    i_CHK,i_CHK2,
    i_MOVEC2, i_MOVE2C, i_CAS, i_CAS2, i_DIVL, i_MULL,
    i_BFTST,i_BFEXTU,i_BFCHG,i_BFEXTS,i_BFCLR,i_BFFFO,i_BFSET,i_BFINS,
    i_PACK, i_UNPK, i_TAS, i_BKPT, i_CALLM, i_RTM, i_TRAPcc, i_MOVES,
    i_FPP, i_FDBcc, i_FScc, i_FTRAPcc, i_FBcc, i_FSAVE, i_FRESTORE,
    i_MMUOP
} instrmnem;

extern struct mnemolookup {
    instrmnem mnemo;
    const char *name;
} lookuptab[];

typedef enum {
    sz_unknown, sz_byte, sz_word, sz_long
} wordsizes;

typedef enum {
    bit0, bit1, bitc, bitC, bitf, biti, bitI, bitj, bitJ, bitk, bitK,
    bits, bitS, bitd, bitD, bitr, bitR, bitz, lastbit
} bitvals;

struct instr_def {
    UWORD bits;
    int n_variable;
    char bitpos[16];
    UWORD mask;
    int plevel;
    const char *opcstr;
};

extern struct instr_def defs68k[];
extern int n_defs68k;

extern struct instr {
    unsigned int mnemo:8;
    unsigned int dmode:5, smode:5;
    unsigned int size:2;
    unsigned int suse:1;
    unsigned int duse:1;
    unsigned int stype:3;
    unsigned int cc:4;
    unsigned int plev:2;
    unsigned char dreg;
    unsigned char sreg;
    signed char dpos;
    signed char spos;
    long int handler;
} *table68k;

extern void read_table68k (void);
extern void do_merges (void);
extern int get_no_mismatches (void);

#endif
#ifndef MUSASHI000_H
#define MUSASHI000_H

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

/* Assembler core - musashik_regs in Assembler routine */
#ifdef A68KEM
extern musashik_regstruct musashik_regs;
#endif

/* ASG 971105 */
typedef struct
{
	musashik_regstruct musashik_regs;
	int pending_interrupts;
} MC_MUSASHI000_Regs;

extern void MC_MUSASHI000_Reset(void);                      /* ASG 971105 */
extern int  MC_MUSASHI000_Execute(int);                     /* ASG 971105 */

#if defined(__cplusplus) && !defined(USE_CYCLONE)
extern "C" {
#endif
extern void MC_MUSASHI000_SetRegs(MC_MUSASHI000_Regs *);          /* ASG 971105 */
extern void MC_MUSASHI000_GetRegs(MC_MUSASHI000_Regs *);          /* ASG 971105 */
#if defined(__cplusplus) && !defined(USE_CYCLONE)
}
#endif

extern void MC_MUSASHI000_Cause_Interrupt(int);             /* ASG 971105 */
extern void MC_MUSASHI000_Clear_Pending_Interrupts(void);   /* ASG 971105 */
extern int  MC_MUSASHI000_GetPC(void);                      /* ASG 971105 */


extern int MC_MUSASHI000_ICount;


#define MC_MUSASHI000_INT_NONE 0							  /* ASG 971105 */
#define MC_MUSASHI000_IRQ_1    1
#define MC_MUSASHI000_IRQ_2    2
#define MC_MUSASHI000_IRQ_3    3
#define MC_MUSASHI000_IRQ_4    4
#define MC_MUSASHI000_IRQ_5    5
#define MC_MUSASHI000_IRQ_6    6
#define MC_MUSASHI000_IRQ_7    7

#define MC_MUSASHI000_STOP     0x10

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif

#endif
void op_0(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_18(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_28(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_30(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_38(void) /* OR */
{
	BYTE src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_39(void) /* OR */
{
	BYTE src = nextiword();
	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_3c(void) /* ORSR */
{
{	MakeSR();
{	WORD src = nextiword();
	src &= 0xFF;
	musashik_regs.sr |= src;
	MakeFromSR();
}}}

void op_40(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_50(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_58(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	musashik_regs.a[dstreg] += 2;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_60(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_68(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_70(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_78(void) /* OR */
{
	WORD src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_79(void) /* OR */
{
	WORD src = nextiword();
	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_7c(void) /* ORSR */
{
	if (!musashik_regs.s)
		{
		musashik_regs.pc -= 2;
		Exception(8,0);
		}
	else
		{
		MakeSR();
			{	WORD src = nextiword();
		musashik_regs.sr |= src;
		MakeFromSR();
			}
		}
}

void op_80(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_90(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_98(void) /* OR */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	musashik_regs.a[dstreg] += 4;
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_a0(void) /* OR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}

void op_a8(void) /* OR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b0(void) /* OR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b8(void) /* OR */
{
{{	LONG src = nextilong();
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b9(void) /* OR */
{
{{	LONG src = nextilong();
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}

void op_100(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
	LONG src = musashik_regs.d[srcreg].D;
	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
}

void op_108(void) /* MVPMR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR memp = musashik_regs.a[srcreg] + nextiword();
	UWORD val = (get_byte(memp) << 8) + get_byte(memp + 2);
	musashik_regs.d[dstreg].W.l = val;
}

void op_110(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}

void op_118(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_120(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_128(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_130(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_138(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_139(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_13a(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 2;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_13b(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 3;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_13c(void) /* BTST */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = nextiword();
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_140(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	musashik_regs.d[dstreg].D = (dst);
}}}}
void op_148(void) /* MVPMR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR memp = musashik_regs.a[srcreg] + nextiword();
{	ULONG val = (get_byte(memp) << 24) + (get_byte(memp + 2) << 16)
							+ (get_byte(memp + 4) << 8) + get_byte(memp + 6);
	musashik_regs.d[dstreg].D = (val);
}}}
void op_150(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_158(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_160(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_168(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_170(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_178(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_179(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_17a(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 2;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_17b(void) /* BCHG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 3;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_180(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	musashik_regs.d[dstreg].D = (dst);
}}}}
void op_188(void) /* MVPRM */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
	CPTR memp = musashik_regs.a[dstreg] + nextiword();
	put_byte(memp, src >> 8); put_byte(memp + 2, src);
}}}
void op_190(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_198(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1a0(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1a8(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_1b0(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1b8(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_1b9(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_1ba(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 2;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1bb(void) /* BCLR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 3;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1c0(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	musashik_regs.d[dstreg].D = (dst);
}}}}
void op_1c8(void) /* MVPRM */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
	CPTR memp = musashik_regs.a[dstreg] + nextiword();
	put_byte(memp, src >> 24); put_byte(memp + 2, src >> 16);
	put_byte(memp + 4, src >> 8); put_byte(memp + 6, src);
}}}
void op_1d0(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_1d8(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1e0(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1e8(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_1f0(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1f8(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_1f9(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_1fa(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 2;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_1fb(void) /* BSET */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg;
	dstreg = 3;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_200(void) /* AND */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_210(void) /* AND */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_218(void) /* AND */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_220(void) /* AND */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_228(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_230(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_238(void) /* AND */
{
{{	BYTE src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_239(void) /* AND */
{
{{	BYTE src = nextiword();
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_23c(void) /* ANDSR */
{
{	MakeSR();
{	WORD src = nextiword();
	src |= 0xFF00;
	musashik_regs.sr &= src;
	MakeFromSR();
}}}
void op_240(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_250(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_258(void) /* AND */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	musashik_regs.a[dstreg] += 2;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_260(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_268(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_270(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_278(void) /* AND */
{
{{	WORD src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_279(void) /* AND */
{
{{	WORD src = nextiword();
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_27c(void) /* ANDSR */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{	MakeSR();
{	WORD src = nextiword();
	musashik_regs.sr &= src;
	MakeFromSR();
}}}}
void op_280(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_290(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_298(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_2a0(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_2a8(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_2b0(void) /* AND */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_2b8(void) /* AND */
{
{{	LONG src = nextilong();
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_2b9(void) /* AND */
{
{{	LONG src = nextilong();
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_400(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_410(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_418(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_420(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_428(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_430(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_438(void) /* SUB */
{
	BYTE src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_439(void) /* SUB */
{
	BYTE src = nextiword();
	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_440(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_450(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_458(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	musashik_regs.a[dstreg] += 2;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}

void op_460(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_468(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_470(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_478(void) /* SUB */
{
	WORD src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_479(void) /* SUB */
{
	WORD src = nextiword();
	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_480(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_490(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_498(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
	musashik_regs.a[dstreg] += 4;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_4a0(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_4a8(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_4b0(void) /* SUB */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_4b8(void) /* SUB */
{
	LONG src = nextilong();
	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_4b9(void) /* SUB */
{
	LONG src = nextilong();
	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_600(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}

void op_610(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}

void op_618(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}}

void op_620(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}}

void op_628(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}

void op_630(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}

void op_638(void) /* ADD */
{
	BYTE src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}

void op_639(void) /* ADD */
{
	BYTE src = nextiword();
	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
}}

void op_640(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}

void op_650(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}

void op_658(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	musashik_regs.a[dstreg] += 2;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}

void op_660(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}}

void op_668(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}

void op_670(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}

void op_678(void) /* ADD */
{
	WORD src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}

void op_679(void) /* ADD */
{
	WORD src = nextiword();
	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
}}

void op_680(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}

void op_690(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}

void op_698(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	musashik_regs.a[dstreg] += 4;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}

void op_6a0(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}}

void op_6a8(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}

void op_6b0(void) /* ADD */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}

void op_6b8(void) /* ADD */
{
	LONG src = nextilong();
	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}

void op_6b9(void) /* ADD */
{
	LONG src = nextilong();
	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
}}

void op_800(void) /* BTST */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
}

void op_810(void) /* BTST */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}

void op_818(void) /* BTST */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
}

void op_820(void) /* BTST */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}

void op_828(void) /* BTST */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_830(void) /* BTST */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_838(void) /* BTST */
{
{{	WORD src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_839(void) /* BTST */
{
{{	WORD src = nextiword();
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_83a(void) /* BTST */
{
	ULONG dstreg;
	dstreg = 2;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_83b(void) /* BTST */
{
	ULONG dstreg;
	dstreg = 3;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}}
void op_83c(void) /* BTST */
{
{{	WORD src = nextiword();
{	BYTE dst = nextiword();
	src &= 7;
	ZFLG = !(dst & (1 << src));
}}}}
void op_840(void) /* BCHG */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	musashik_regs.d[dstreg].D = (dst);
}}}}
void op_850(void) /* BCHG */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_858(void) /* BCHG */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_860(void) /* BCHG */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_868(void) /* BCHG */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_870(void) /* BCHG */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_878(void) /* BCHG */
{
{{	WORD src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_879(void) /* BCHG */
{
{{	WORD src = nextiword();
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_87a(void) /* BCHG */
{
	ULONG dstreg;
	dstreg = 2;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_87b(void) /* BCHG */
{
	ULONG dstreg;
	dstreg = 3;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst ^= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_880(void) /* BCLR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	musashik_regs.d[dstreg].D = (dst);
}}}}
void op_890(void) /* BCLR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_898(void) /* BCLR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8a0(void) /* BCLR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8a8(void) /* BCLR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_8b0(void) /* BCLR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8b8(void) /* BCLR */
{
{{	WORD src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_8b9(void) /* BCLR */
{
{{	WORD src = nextiword();
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}
void op_8ba(void) /* BCLR */
{
	ULONG dstreg;
	dstreg = 2;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8bb(void) /* BCLR */
{
	ULONG dstreg;
	dstreg = 3;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst &= ~(1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8c0(void) /* BSET */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= 31;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	musashik_regs.d[dstreg].D = (dst);
}}}}
void op_8d0(void) /* BSET */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_8d8(void) /* BSET */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8e0(void) /* BSET */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8e8(void) /* BSET */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_8f0(void) /* BSET */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8f8(void) /* BSET */
{
{{	WORD src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_8f9(void) /* BSET */
{
{{	WORD src = nextiword();
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}
void op_8fa(void) /* BSET */
{
	ULONG dstreg;
	dstreg = 2;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_8fb(void) /* BSET */
{
	ULONG dstreg;
	dstreg = 3;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_getpc());
{	BYTE dst = get_byte(dsta);
	src &= 7;
	ZFLG = !(dst & (1 << src));
	dst |= (1 << src);
	put_byte(dsta,dst);
}}}}}
void op_a00(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src ^= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a10(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a18(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_a20(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_a28(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a30(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	BYTE src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_a38(void) /* EOR */
{
{{	BYTE src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a39(void) /* EOR */
{
{{	BYTE src = nextiword();
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a3c(void) /* EORSR */
{
{	MakeSR();
{	WORD src = nextiword();
	src &= 0xFF;
	musashik_regs.sr ^= src;
	MakeFromSR();
}}}
void op_a40(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src ^= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a50(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_a58(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	musashik_regs.a[dstreg] += 2;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_a60(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_a68(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a70(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_a78(void) /* EOR */
{
{{	WORD src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a79(void) /* EOR */
{
{{	WORD src = nextiword();
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a7c(void) /* EORSR */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{	MakeSR();
{	WORD src = nextiword();
	musashik_regs.sr ^= src;
	MakeFromSR();
}}}}
void op_a80(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.d[dstreg].D;
	src ^= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a90(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_a98(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_aa0(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_aa8(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_ab0(void) /* EOR */
{
	ULONG dstreg = opcode & 7;
{{	LONG src = nextilong();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_ab8(void) /* EOR */
{
{{	LONG src = nextilong();
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_ab9(void) /* EOR */
{
	LONG src = nextilong();
	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_c00(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_c10(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_c18(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_c20(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_c28(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_c30(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	BYTE src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_c38(void) /* CMP */
{
	BYTE src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_c39(void) /* CMP */
{
	BYTE src = nextiword();
	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_c3a(void) /* CMP */
{
	BYTE src = nextiword();
	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_c3b(void) /* CMP */
{
	BYTE src = nextiword();
	CPTR dsta = get_disp_ea(musashik_getpc());
	BYTE dst = get_byte(dsta);
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_c40(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_c50(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_c58(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	musashik_regs.a[dstreg] += 2;
{	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_c60(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_c68(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_c70(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	WORD src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_c78(void) /* CMP */
{
	WORD src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_c79(void) /* CMP */
{
	WORD src = nextiword();
	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_c7a(void) /* CMP */
{
	WORD src = nextiword();
	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_c7b(void) /* CMP */
{
	WORD src = nextiword();
	CPTR dsta = get_disp_ea(musashik_getpc());
	WORD dst = get_word(dsta);
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_c80(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_c90(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_c98(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	musashik_regs.a[dstreg] += 4;
{	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_ca0(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_ca8(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_cb0(void) /* CMP */
{
	ULONG dstreg = opcode & 7;
	LONG src = nextilong();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_cb8(void) /* CMP */
{
	LONG src = nextilong();
	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_cb9(void) /* CMP */
{
	LONG src = nextilong();
	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_cba(void) /* CMP */
{
	LONG src = nextilong();
	CPTR dsta = musashik_getpc();
	dsta += (LONG)(WORD)nextiword();
{	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_cbb(void) /* CMP */
{
	LONG src = nextilong();
	CPTR dsta = get_disp_ea(musashik_getpc());
	LONG dst = get_long(dsta);
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}
void op_1000(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1010(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1018(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1020(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1028(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1030(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1038(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1039(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_103a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_103b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_103c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1080(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1090(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1098(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_10a0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_10a8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10b0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10b8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10b9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10ba(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_10bb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10bc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_10e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_10e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10f8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10f9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10fa(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_10fb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_10fc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1100(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1110(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1118(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1120(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}

void op_1128(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1130(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1138(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1139(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_113a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}

void op_113b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_113c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1140(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1150(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1158(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1160(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_1168(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1170(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1178(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1179(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_117a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_117b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_117c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1180(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1190(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_1198(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_11a0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_11a8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11b0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11b8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11b9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11ba(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_11bb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11bc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_11e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_11e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11f8(void) /* MOVE */
{
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11f9(void) /* MOVE */
{
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
	put_byte(dsta,src);
}

void op_11fa(void) /* MOVE */
{
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_11fb(void) /* MOVE */
{
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_11fc(void) /* MOVE */
{
	BYTE src = nextiword();
	CPTR dsta = (LONG)(WORD)nextiword();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_13e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_13e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13f8(void) /* MOVE */
{
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13f9(void) /* MOVE */
{
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13fa(void) /* MOVE */
{
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_13fb(void) /* MOVE */
{
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_13fc(void) /* MOVE */
{
	BYTE src = nextiword();
	CPTR dsta = nextilong();
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}
void op_2000(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2008(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2010(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2018(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2020(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2028(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2030(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2038(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2039(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_203a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_203b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_203c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2040(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	musashik_regs.a[dstreg] = (src);
}

void op_2048(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	musashik_regs.a[dstreg] = (src);
}

void op_2050(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}

void op_2058(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
	musashik_regs.a[dstreg] = (src);
}

void op_2060(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}}

void op_2068(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}

void op_2070(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}

void op_2078(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}

void op_2079(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}

void op_207a(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}}

void op_207b(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] = (src);
}

void op_207c(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	musashik_regs.a[dstreg] = (src);
}

void op_2080(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2088(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2090(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2098(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}
void op_20a0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_20a8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20b0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20b8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20b9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20ba(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_20bb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20bc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20c8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_20e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_20e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20f8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20f9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20fa(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_20fb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_20fc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] += 4;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2100(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2108(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2110(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2118(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2120(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}

void op_2128(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2130(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2138(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2139(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_213a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}

void op_213b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_213c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2140(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2148(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2150(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2158(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2160(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_2168(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2170(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2178(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2179(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_217a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_217b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_217c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2180(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2188(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2190(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_2198(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_21a0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_21a8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21b0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21b8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21b9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21ba(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_21bb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21bc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	LONG src = musashik_regs.d[srcreg].D;
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21c8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	LONG src = musashik_regs.a[srcreg];
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_21e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_21e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21f8(void) /* MOVE */
{
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21f9(void) /* MOVE */
{
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21fa(void) /* MOVE */
{
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_21fb(void) /* MOVE */
{
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_21fc(void) /* MOVE */
{
	LONG src = nextilong();
	CPTR dsta = (LONG)(WORD)nextiword();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	LONG src = musashik_regs.d[srcreg].D;
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23c8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	LONG src = musashik_regs.a[srcreg];
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
{	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_23e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_23e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23f8(void) /* MOVE */
{
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23f9(void) /* MOVE */
{
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23fa(void) /* MOVE */
{
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}

void op_23fb(void) /* MOVE */
{
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_23fc(void) /* MOVE */
{
	LONG src = nextilong();
	CPTR dsta = nextilong();
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}
void op_3000(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3008(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3010(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3018(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3020(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3028(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3030(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3038(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3039(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_303a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_303b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_303c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
	musashik_regs.d[dstreg].W.l = src;
{	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3040(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}
void op_3048(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}
void op_3050(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}
void op_3058(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}}
void op_3060(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}}
void op_3068(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}
void op_3070(void) /* MOVEA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}}
void op_3078(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}
void op_3079(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}
void op_307a(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}}
void op_307b(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}}
void op_307c(void) /* MOVEA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	musashik_regs.a[dstreg] = (LONG)(WORD)(src);
}}}}
void op_3080(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3088(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3090(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3098(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30a0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30a8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_30b0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30b8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_30b9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_30ba(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30bb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30bc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_30c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30c8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_30e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_30e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_30f8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30f9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_30fa(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_30fb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_30fc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg];
{	musashik_regs.a[dstreg] += 2;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3100(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3108(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3110(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3118(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_3120(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_3128(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3130(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_3138(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3139(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_313a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_313b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}}
void op_313c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3140(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3148(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3150(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3158(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3160(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3168(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3170(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_3178(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3179(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_317a(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_317b(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_317c(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3180(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3188(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3190(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_3198(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31a0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31a8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31b0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31b8(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31b9(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31ba(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31bb(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31bc(void) /* MOVE */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31c8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	WORD src = musashik_regs.a[srcreg];
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31f8(void) /* MOVE */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31f9(void) /* MOVE */
{
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_31fa(void) /* MOVE */
{
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31fb(void) /* MOVE */
{
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_31fc(void) /* MOVE */
{
{{	WORD src = nextiword();
{	CPTR dsta = (LONG)(WORD)nextiword();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_33c0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_33c8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	WORD src = musashik_regs.a[srcreg];
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_33d0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_33d8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_33e0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_33e8(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_33f0(void) /* MOVE */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_33f8(void) /* MOVE */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_33f9(void) /* MOVE */
{
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
void op_33fa(void) /* MOVE */
{
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_33fb(void) /* MOVE */
{
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}}
void op_33fc(void) /* MOVE */
{
{{	WORD src = nextiword();
{	CPTR dsta = nextilong();
	put_word(dsta,src);
	CLEARFLGS;
	if ( src == 0 )
		ZFLG = ZTRUE ;
	if ( src < 0 )
		NFLG = NTRUE ;
}}}}
extern int pending_interrupts;

void op_4000(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      BYTE src = musashik_regs.d[srcreg].B.l;
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        musashik_regs.d[srcreg].B.l = newv;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}
void op_4010(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_byte(srca,newv);
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}
void op_4018(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_byte(srca,newv);
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}}
void op_4020(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{       CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_byte(srca,newv);
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}}
void op_4028(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_byte(srca,newv);
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}
void op_4030(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       BYTE src = get_byte(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_byte(srca,newv);
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}}
void op_4038(void) /* NEGX */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_byte(srca,newv);
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}
void op_4039(void) /* NEGX */
{
{{      CPTR srca = nextilong();
        BYTE src = get_byte(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_byte(srca,newv);
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
}}}}}
void op_4040(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      WORD src = musashik_regs.d[srcreg].W.l;
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        musashik_regs.d[srcreg].W.l = newv;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}
void op_4050(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_word(srca,newv);
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}
void op_4058(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       musashik_regs.a[srcreg] += 2;
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_word(srca,newv);
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}}
void op_4060(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_word(srca,newv);
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}}
void op_4068(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_word(srca,newv);
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}
void op_4070(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       WORD src = get_word(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_word(srca,newv);
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}}
void op_4078(void) /* NEGX */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_word(srca,newv);
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}
void op_4079(void) /* NEGX */
{
{{      CPTR srca = nextilong();
        WORD src = get_word(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_word(srca,newv);
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((WORD)(newv)) != 0) ZFLG = 0;
        NFLG = ((WORD)(newv)) < 0;
}}}}}
void op_4080(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        musashik_regs.d[srcreg].D = (newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}
void op_4090(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_long(srca,newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}
void op_4098(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       musashik_regs.a[srcreg] += 4;
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_long(srca,newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}}
void op_40a0(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 4;
{       CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_long(srca,newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}}
void op_40a8(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_long(srca,newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}
void op_40b0(void) /* NEGX */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       LONG src = get_long(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_long(srca,newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}}
void op_40b8(void) /* NEGX */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_long(srca,newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}
void op_40b9(void) /* NEGX */
{
{{      CPTR srca = nextilong();
        LONG src = get_long(srca);
{       ULONG newv = 0 - src - (musashik_regs.x ? 1 : 0);
        put_long(srca,newv);
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(newv)) < 0;
        VFLG = (flgs && flgn);
        musashik_regs.x = CFLG = (flgs || flgn);
        if (((LONG)(newv)) != 0) ZFLG = 0;
        NFLG = ((LONG)(newv)) < 0;
}}}}}
void op_40c0(void) /* MVSR2 */
{
        ULONG srcreg = (opcode & 7);
{{      MakeSR();
        musashik_regs.d[srcreg].W.l = musashik_regs.sr;
}}}
void op_40d0(void) /* MVSR2 */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        MakeSR();
        put_word(srca,musashik_regs.sr);
}}}
void op_40d8(void) /* MVSR2 */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
{       musashik_regs.a[srcreg] += 2;
        MakeSR();
        put_word(srca,musashik_regs.sr);
}}}}
void op_40e0(void) /* MVSR2 */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        MakeSR();
        put_word(srca,musashik_regs.sr);
}}}}
void op_40e8(void) /* MVSR2 */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        MakeSR();
        put_word(srca,musashik_regs.sr);
}}}
void op_40f0(void) /* MVSR2 */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
        MakeSR();
        put_word(srca,musashik_regs.sr);
}}}
void op_40f8(void) /* MVSR2 */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        MakeSR();
        put_word(srca,musashik_regs.sr);
}}}
void op_40f9(void) /* MVSR2 */
{
{{      CPTR srca = nextilong();
        MakeSR();
        put_word(srca,musashik_regs.sr);
}}}
void op_4100(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       LONG src = musashik_regs.d[srcreg].D;
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_4110(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_4118(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       musashik_regs.a[srcreg] += 4;
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_4120(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       musashik_regs.a[srcreg] -= 4;
{       CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_4128(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_4130(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_4138(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_4139(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = nextilong();
        LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_413a(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_413b(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = get_disp_ea(musashik_getpc());
{       LONG src = get_long(srca);
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_413c(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       LONG src = nextilong();
{       LONG dst = musashik_regs.d[dstreg].D;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_4180(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       WORD src = musashik_regs.d[srcreg].W.l;
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_4190(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_4198(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       musashik_regs.a[srcreg] += 2;
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_41a0(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_41a8(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_41b0(void) /* CHK */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_41b8(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_41b9(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = nextilong();
        WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_41ba(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_41bb(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       CPTR srca = get_disp_ea(musashik_getpc());
{       WORD src = get_word(srca);
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}}
void op_41bc(void) /* CHK */
{
        ULONG dstreg = (opcode >> 9) & 7;
{       CPTR oldpc = musashik_getpc();
{       WORD src = nextiword();
{       WORD dst = musashik_regs.d[dstreg].W.l;
        if ((LONG)dst < 0) { NFLG=1; Exception(6,oldpc-2); }
        else if (dst > src) { NFLG=0; Exception(6,oldpc-2); }
}}}}
void op_41d0(void) /* LEA */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{{      CPTR srca = musashik_regs.a[srcreg];
{       musashik_regs.a[dstreg] = (srca);
}}}}
void op_41e8(void) /* LEA */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{       musashik_regs.a[dstreg] = (srca);
}}}}
void op_41f0(void) /* LEA */
{
        ULONG srcreg = (opcode & 7);
        ULONG dstreg = (opcode >> 9) & 7;
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       musashik_regs.a[dstreg] = (srca);
}}}}
void op_41f8(void) /* LEA */
{
        ULONG dstreg = (opcode >> 9) & 7;
{{      CPTR srca = (LONG)(WORD)nextiword();
{       musashik_regs.a[dstreg] = (srca);
}}}}
void op_41f9(void) /* LEA */
{
        ULONG dstreg = (opcode >> 9) & 7;
{{      CPTR srca = nextilong();
{       musashik_regs.a[dstreg] = (srca);
}}}}
void op_41fa(void) /* LEA */
{
        ULONG dstreg = (opcode >> 9) & 7;
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       musashik_regs.a[dstreg] = (srca);
}}}}
void op_41fb(void) /* LEA */
{
        ULONG dstreg = (opcode >> 9) & 7;
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       musashik_regs.a[dstreg] = (srca);
}}}}
void op_4200(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
        musashik_regs.d[srcreg].B.l = 0;
{{      CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4210(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        put_byte(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4218(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
{       musashik_regs.a[srcreg] += areg_byteinc[srcreg];
        put_byte(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}}
void op_4220(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{       CPTR srca = musashik_regs.a[srcreg];
        put_byte(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}}
void op_4228(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        put_byte(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4230(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
        put_byte(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4238(void) /* CLR */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        put_byte(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4239(void) /* CLR */
{
{{      CPTR srca = nextilong();
        put_byte(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4240(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
        musashik_regs.d[srcreg].W.l = 0;
{{      CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4250(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        put_word(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4258(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
{       musashik_regs.a[srcreg] += 2;
        put_word(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}}
void op_4260(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        put_word(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}}
void op_4268(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        put_word(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4270(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
        put_word(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4278(void) /* CLR */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        put_word(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4279(void) /* CLR */
{
{{      CPTR srca = nextilong();
        put_word(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4280(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
        musashik_regs.d[srcreg].D = (0);
{{      CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4290(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        put_long(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4298(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
{       musashik_regs.a[srcreg] += 4;
        put_long(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}}
void op_42a0(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 4;
{       CPTR srca = musashik_regs.a[srcreg];
        put_long(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}}
void op_42a8(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        put_long(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_42b0(void) /* CLR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
        put_long(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_42b8(void) /* CLR */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        put_long(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_42b9(void) /* CLR */
{
{{      CPTR srca = nextilong();
        put_long(srca,0);
        CLEARFLGS;
        ZFLG = ZTRUE ;
}}}
void op_4400(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      BYTE src = musashik_regs.d[srcreg].B.l;
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        musashik_regs.d[srcreg].B.l = dst;
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4410(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        put_byte(srca,dst);
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4418(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        put_byte(srca,dst);
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_4420(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{       CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        put_byte(srca,dst);
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_4428(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        put_byte(srca,dst);
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4430(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       BYTE src = get_byte(srca);
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        put_byte(srca,dst);
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_4438(void) /* NEG */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        put_byte(srca,dst);
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4439(void) /* NEG */
{
{{      CPTR srca = nextilong();
        BYTE src = get_byte(srca);
{{ULONG dst = ((BYTE)(0)) - ((BYTE)(src));
        put_byte(srca,dst);
        ZFLG = ((BYTE)(dst)) == 0;
{       int flgs = ((BYTE)(src)) < 0;
        int flgn = ((BYTE)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4440(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      WORD src = musashik_regs.d[srcreg].W.l;
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        musashik_regs.d[srcreg].W.l = dst;
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4450(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        put_word(srca,dst);
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4458(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       musashik_regs.a[srcreg] += 2;
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        put_word(srca,dst);
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_4460(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        put_word(srca,dst);
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_4468(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        put_word(srca,dst);
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4470(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       WORD src = get_word(srca);
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        put_word(srca,dst);
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_4478(void) /* NEG */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        put_word(srca,dst);
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4479(void) /* NEG */
{
{{      CPTR srca = nextilong();
        WORD src = get_word(srca);
{{ULONG dst = ((WORD)(0)) - ((WORD)(src));
        put_word(srca,dst);
        ZFLG = ((WORD)(dst)) == 0;
{       int flgs = ((WORD)(src)) < 0;
        int flgn = ((WORD)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4480(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        musashik_regs.d[srcreg].D = (dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4490(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        put_long(srca,dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}
void op_4498(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       musashik_regs.a[srcreg] += 4;
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        put_long(srca,dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_44a0(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 4;
{       CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        put_long(srca,dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_44a8(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        put_long(srca,dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}
void op_44b0(void) /* NEG */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       LONG src = get_long(srca);
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        put_long(srca,dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}}
void op_44b8(void) /* NEG */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        put_long(srca,dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}
void op_44b9(void) /* NEG */
{
{{      CPTR srca = nextilong();
        LONG src = get_long(srca);
{{ULONG dst = ((LONG)(0)) - ((LONG)(src));
        put_long(srca,dst);
        ZFLG = ((LONG)(dst)) == 0;
{       int flgs = ((LONG)(src)) < 0;
        int flgn = ((LONG)(dst)) < 0;
        VFLG = (flgs && flgn);
        CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(0));
        NFLG = flgn != 0;
}}}}}}
void op_44c0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{{      WORD src = musashik_regs.d[srcreg].W.l;
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}
void op_44d0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}
void op_44d8(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       musashik_regs.a[srcreg] += 2;
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}}
void op_44e0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}}
void op_44e8(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}
void op_44f0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}}
void op_44f8(void) /* MV2SR */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}
void op_44f9(void) /* MV2SR */
{
{{      CPTR srca = nextilong();
        WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}
void op_44fa(void) /* MV2SR */
{
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}}
void op_44fb(void) /* MV2SR */
{
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       WORD src = get_word(srca);
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}}
void op_44fc(void) /* MV2SR */
{
{{      WORD src = nextiword();
        MakeSR();
        musashik_regs.sr &= 0xFF00;
        musashik_regs.sr |= src & 0xFF;
        MakeFromSR();
}}}
void op_4600(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      BYTE src = musashik_regs.d[srcreg].B.l;
{       ULONG dst = ~src;
        musashik_regs.d[srcreg].B.l = dst;
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4610(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       ULONG dst = ~src;
        put_byte(srca,dst);
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4618(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{       ULONG dst = ~src;
        put_byte(srca,dst);
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_4620(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{       CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       ULONG dst = ~src;
        put_byte(srca,dst);
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_4628(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{       ULONG dst = ~src;
        put_byte(srca,dst);
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4630(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       BYTE src = get_byte(srca);
{       ULONG dst = ~src;
        put_byte(srca,dst);
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_4638(void) /* NOT */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{       ULONG dst = ~src;
        put_byte(srca,dst);
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4639(void) /* NOT */
{
{{      CPTR srca = nextilong();
        BYTE src = get_byte(srca);
{       ULONG dst = ~src;
        put_byte(srca,dst);
        CLEARFLGS;
		if ((BYTE)dst <= 0 )
		{
			if ((BYTE)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4640(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      WORD src = musashik_regs.d[srcreg].W.l;
{       ULONG dst = ~src;
        musashik_regs.d[srcreg].W.l = dst;
        CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
			if ((WORD)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4650(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       ULONG dst = ~src;
        put_word(srca,dst);
        CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
			if ((WORD)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4658(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       musashik_regs.a[srcreg] += 2;
{       ULONG dst = ~src;
        put_word(srca,dst);
        CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
			if ((WORD)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_4660(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       ULONG dst = ~src;
        put_word(srca,dst);
        CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
			if ((WORD)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_4668(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{       ULONG dst = ~src;
        put_word(srca,dst);
        CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
		if ((WORD)dst == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4670(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       WORD src = get_word(srca);
{       ULONG dst = ~src;
        put_word(srca,dst);
        CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
			if ((WORD)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_4678(void) /* NOT */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
{       ULONG dst = ~src;
        put_word(srca,dst);
	    CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
			if ((WORD)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4679(void) /* NOT */
{
{{      CPTR srca = nextilong();
        WORD src = get_word(srca);
{       ULONG dst = ~src;
        put_word(srca,dst);
        CLEARFLGS;
		if ((WORD)dst <= 0 )
		{
			if ((WORD)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4680(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
{       ULONG dst = ~src;
        musashik_regs.d[srcreg].D = (dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4690(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       ULONG dst = ~src;
        put_long(srca,dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_4698(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       musashik_regs.a[srcreg] += 4;
{       ULONG dst = ~src;
        put_long(srca,dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_46a0(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 4;
{       CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       ULONG dst = ~src;
        put_long(srca,dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_46a8(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{       ULONG dst = ~src;
        put_long(srca,dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_46b0(void) /* NOT */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       LONG src = get_long(srca);
{       ULONG dst = ~src;
        put_long(srca,dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}}
void op_46b8(void) /* NOT */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
{       ULONG dst = ~src;
        put_long(srca,dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_46b9(void) /* NOT */
{
{{      CPTR srca = nextilong();
        LONG src = get_long(srca);
{       ULONG dst = ~src;
        put_long(srca,dst);
        CLEARFLGS;
		if ((LONG)dst <= 0 )
		{
			if ((LONG)dst == 0 )
				ZFLG = ZTRUE ;
			else
				NFLG = NTRUE ;
		}
}}}}
void op_46c0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      WORD src = musashik_regs.d[srcreg].W.l;
        musashik_regs.sr = src;
        MakeFromSR();
}}}}
void op_46d0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}
void op_46d8(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       musashik_regs.a[srcreg] += 2;
        musashik_regs.sr = src;
        MakeFromSR();
}}}}}
void op_46e0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}}
void op_46e8(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}
void op_46f0(void) /* MV2SR */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}}
void op_46f8(void) /* MV2SR */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}
void op_46f9(void) /* MV2SR */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = nextilong();
        WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}
void op_46fa(void) /* MV2SR */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}}
void op_46fb(void) /* MV2SR */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       WORD src = get_word(srca);
        musashik_regs.sr = src;
        MakeFromSR();
}}}}}
void op_46fc(void) /* MV2SR */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      WORD src = nextiword();
        musashik_regs.sr = src;
        MakeFromSR();
}}}}
void op_4800(void) /* NBCD */
{
        ULONG srcreg = (opcode & 7);
{{      BYTE src = musashik_regs.d[srcreg].B.l;
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        musashik_regs.d[srcreg].B.l = newv;
}}}}
void op_4810(void) /* NBCD */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        put_byte(srca,newv);
}}}}
void op_4818(void) /* NBCD */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        put_byte(srca,newv);
}}}}}
void op_4820(void) /* NBCD */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{       CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        put_byte(srca,newv);
}}}}}
void op_4828(void) /* NBCD */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        put_byte(srca,newv);
}}}}
void op_4830(void) /* NBCD */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       BYTE src = get_byte(srca);
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        put_byte(srca,newv);
}}}}}
void op_4838(void) /* NBCD */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        put_byte(srca,newv);
}}}}
void op_4839(void) /* NBCD */
{
{{      CPTR srca = nextilong();
        BYTE src = get_byte(srca);
{       UWORD newv_lo = - (src & 0xF) - (musashik_regs.x ? 1 : 0);
        UWORD newv_hi = - (src & 0xF0);
        UWORD newv;
        if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
        newv = newv_hi + (newv_lo & 0xF);       CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
        if (CFLG) newv -= 0x60;
        if (((BYTE)(newv)) != 0) ZFLG = 0;
        NFLG = ((BYTE)(newv)) < 0;
        put_byte(srca,newv);
}}}}
void op_4840(void) /* SWAP */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
{       ULONG dst = ((src >> 16)&0xFFFF) | ((src&0xFFFF)<<16);
        CLEARFLGS;
        ZFLG = ((LONG)(dst)) == 0;
        NFLG = ((LONG)(dst)) < 0;
        musashik_regs.d[srcreg].D = (dst);
}}}}
void op_4850(void) /* PEA */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
{       musashik_regs.a[7] -= 4;
{       CPTR dsta = musashik_regs.a[7];
        put_long(dsta,srca);
}}}}}
void op_4868(void) /* PEA */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{       musashik_regs.a[7] -= 4;
{       CPTR dsta = musashik_regs.a[7];
        put_long(dsta,srca);
}}}}}
void op_4870(void) /* PEA */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       musashik_regs.a[7] -= 4;
{       CPTR dsta = musashik_regs.a[7];
        put_long(dsta,srca);
}}}}}
void op_4878(void) /* PEA */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
{       musashik_regs.a[7] -= 4;
{       CPTR dsta = musashik_regs.a[7];
        put_long(dsta,srca);
}}}}}
void op_4879(void) /* PEA */
{
{{      CPTR srca = nextilong();
{       musashik_regs.a[7] -= 4;
{       CPTR dsta = musashik_regs.a[7];
        put_long(dsta,srca);
}}}}}
void op_487a(void) /* PEA */
{
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       musashik_regs.a[7] -= 4;
{       CPTR dsta = musashik_regs.a[7];
        put_long(dsta,srca);
}}}}}
void op_487b(void) /* PEA */
{
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       musashik_regs.a[7] -= 4;
{       CPTR dsta = musashik_regs.a[7];
        put_long(dsta,srca);
}}}}}
void op_4880(void) /* EXT */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
{       UWORD dst = (WORD)(BYTE)src;
        CLEARFLGS;
        ZFLG = ((WORD)(dst)) == 0;
        NFLG = ((WORD)(dst)) < 0;
        musashik_regs.d[srcreg].W.l = dst;
}}}}
void op_4890(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{       CPTR srca = musashik_regs.a[dstreg];
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_word(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { put_word(srca, musashik_regs.a[movem_index1[amask]]); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_48a0(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{{      CPTR srca = musashik_regs.a[dstreg];
{       UWORD amask = mask & 0xff, dmask = (mask >> 8) & 0xff;
        while (amask) { srca -= 2; put_word(srca, musashik_regs.a[movem_index2[amask]]); amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
	while (dmask) { srca -= 2; put_word(srca, musashik_regs.d[movem_index2[dmask]].D); dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        musashik_regs.a[dstreg] = srca;
}}}}}
void op_48a8(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{       CPTR srca = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_word(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { put_word(srca, musashik_regs.a[movem_index1[amask]]); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_48b0(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{       CPTR srca = get_disp_ea(musashik_regs.a[dstreg]);
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_word(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { put_word(srca, musashik_regs.a[movem_index1[amask]]); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_48b8(void) /* MVMLE */
{
{       UWORD mask = nextiword();
{       CPTR srca = (LONG)(WORD)nextiword();
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_word(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { put_word(srca, musashik_regs.a[movem_index1[amask]]); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_48b9(void) /* MVMLE */
{
{       UWORD mask = nextiword();
{       CPTR srca = nextilong();
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_word(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { put_word(srca, musashik_regs.a[movem_index1[amask]]); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_48c0(void) /* EXT */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
{       ULONG dst = (LONG)(WORD)src;
        CLEARFLGS;
        ZFLG = ((LONG)(dst)) == 0;
        NFLG = ((LONG)(dst)) < 0;
        musashik_regs.d[srcreg].D = (dst);
}}}}
void op_48d0(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{       CPTR srca = musashik_regs.a[dstreg];
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_long(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
        while (amask) { put_long(srca, musashik_regs.a[movem_index1[amask]]); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_48e0(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{{      CPTR srca = musashik_regs.a[dstreg];
{       UWORD amask = mask & 0xff, dmask = (mask >> 8) & 0xff;
        while (amask) { srca -= 4; put_long(srca, musashik_regs.a[movem_index2[amask]]); amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
	while (dmask) { srca -= 4; put_long(srca, musashik_regs.d[movem_index2[dmask]].D); dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
        musashik_regs.a[dstreg] = srca;
}}}}}
void op_48e8(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{       CPTR srca = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_long(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
        while (amask) { put_long(srca, musashik_regs.a[movem_index1[amask]]); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_48f0(void) /* MVMLE */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword();
{       CPTR srca = get_disp_ea(musashik_regs.a[dstreg]);
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_long(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
        while (amask) { put_long(srca, musashik_regs.a[movem_index1[amask]]); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_48f8(void) /* MVMLE */
{
{       UWORD mask = nextiword();
{       CPTR srca = (LONG)(WORD)nextiword();
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_long(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
        while (amask) { put_long(srca, musashik_regs.a[movem_index1[amask]]); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_48f9(void) /* MVMLE */
{
{       UWORD mask = nextiword();
{       CPTR srca = nextilong();
{       UWORD dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
	while (dmask) { put_long(srca, musashik_regs.d[movem_index1[dmask]].D); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
        while (amask) { put_long(srca, musashik_regs.a[movem_index1[amask]]); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_49c0(void) /* EXT */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
{       ULONG dst = (LONG)(BYTE)src;
        musashik_regs.d[srcreg].D = (dst);
        CLEARFLGS;
	if ( (LONG)dst <= 0 )
		{
		if ( dst == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a00(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      BYTE src = musashik_regs.d[srcreg].B.l;
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a10(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a18(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       musashik_regs.a[srcreg] += areg_byteinc[srcreg];
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a20(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{       CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a28(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a30(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a38(void) /* TST */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a39(void) /* TST */
{
{{      CPTR srca = nextilong();
        BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a3a(void) /* TST */
{
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a3b(void) /* TST */
{
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       BYTE src = get_byte(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a3c(void) /* TST */
{
{{      BYTE src = nextiword();
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a40(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      WORD src = musashik_regs.d[srcreg].W.l;
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a48(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      WORD src = musashik_regs.a[srcreg];
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a50(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a58(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
{       musashik_regs.a[srcreg] += 2;
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a60(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 2;
{       CPTR srca = musashik_regs.a[srcreg];
        WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a68(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a70(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a78(void) /* TST */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a79(void) /* TST */
{
{{      CPTR srca = nextilong();
        WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a7a(void) /* TST */
{
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a7b(void) /* TST */
{
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       WORD src = get_word(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4a7c(void) /* TST */
{
{{      WORD src = nextiword();
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a80(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.d[srcreg].D;
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a88(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.a[srcreg];
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a90(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4a98(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
{       musashik_regs.a[srcreg] += 4;
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4aa0(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= 4;
{       CPTR srca = musashik_regs.a[srcreg];
        LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4aa8(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4ab0(void) /* TST */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4ab8(void) /* TST */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4ab9(void) /* TST */
{
{{      CPTR srca = nextilong();
        LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4aba(void) /* TST */
{
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4abb(void) /* TST */
{
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       LONG src = get_long(srca);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4abc(void) /* TST */
{
{{      LONG src = nextilong();
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4ac0(void) /* TAS */
{
        ULONG srcreg = (opcode & 7);
{{      BYTE src = musashik_regs.d[srcreg].B.l;
        musashik_regs.d[srcreg].B.l = src | 0x80;
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4ad0(void) /* TAS */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
        put_byte(srca,src | 0x80 );
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4ad8(void) /* TAS */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
{       musashik_regs.a[srcreg] += areg_byteinc[srcreg];
        put_byte(srca,src | 0x80 );
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4ae0(void) /* TAS */
{
        ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{       CPTR srca = musashik_regs.a[srcreg];
        BYTE src = get_byte(srca);
        put_byte(srca,src | 0x80 );
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4ae8(void) /* TAS */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
        put_byte(srca,src | 0x80);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4af0(void) /* TAS */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       BYTE src = get_byte(srca);
        put_byte(srca,src | 0x80);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}}
void op_4af8(void) /* TAS */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        BYTE src = get_byte(srca);
        put_byte(srca,src | 0x80);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4af9(void) /* TAS */
{
{{      CPTR srca = nextilong();
        BYTE src = get_byte(srca);
        put_byte(srca,src | 0x80);
        CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src == 0 )
			ZFLG = ZTRUE ;
		else
			NFLG = NTRUE ;
		}
}}}
void op_4c90(void) /* MVMEL */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_regs.a[dstreg];
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_4c98(void) /* MVMEL */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_regs.a[dstreg];
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
        musashik_regs.a[dstreg] = srca;
}}}}
void op_4ca8(void) /* MVMEL */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_4cb0(void) /* MVMEL */
{
        ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = get_disp_ea(musashik_regs.a[dstreg]);
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_4cb8(void) /* MVMEL */
{
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = (LONG)(WORD)nextiword();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_4cb9(void) /* MVMEL */
{
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = nextilong();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
        while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_4cba(void) /* MVMEL */
{
				ULONG dstreg;
				dstreg = 2;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_getpc();
				srca += (LONG)(WORD)nextiword();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
				while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_4cbb(void) /* MVMEL */
{
				ULONG dstreg;
				dstreg = 3;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = get_disp_ea(musashik_getpc());
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = (LONG)(WORD)get_word(srca); srca += 2; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 4; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = (LONG)(WORD)get_word(srca); srca += 2; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 4; }
}}}}
void op_4cd0(void) /* MVMEL */
{
				ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_regs.a[dstreg];
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_4cd8(void) /* MVMEL */
{
				ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_regs.a[dstreg];
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
	musashik_regs.a[dstreg] = srca;
}}}}
void op_4ce8(void) /* MVMEL */
{
				ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_4cf0(void) /* MVMEL */
{
				ULONG dstreg = opcode & 7;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = get_disp_ea(musashik_regs.a[dstreg]);
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_4cf8(void) /* MVMEL */
{
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = (LONG)(WORD)nextiword();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_4cf9(void) /* MVMEL */
{
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = nextilong();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_4cfa(void) /* MVMEL */
{
				ULONG dstreg;
				dstreg = 2;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = musashik_getpc();
				srca += (LONG)(WORD)nextiword();
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_4cfb(void) /* MVMEL */
{
				ULONG dstreg;
				dstreg = 3;
{       UWORD mask = nextiword(), dmask = mask & 0xff, amask = (mask >> 8) & 0xff;
{       CPTR srca = get_disp_ea(musashik_getpc());
{	while (dmask) { musashik_regs.d[movem_index1[dmask]].D = get_long(srca); srca += 4; dmask = movem_next[dmask]; MC_MUSASHI000_ICount -= 8; }
	while (amask) { musashik_regs.a[movem_index1[amask]] = get_long(srca); srca += 4; amask = movem_next[amask]; MC_MUSASHI000_ICount -= 8; }
}}}}
void op_4e40(void) /* TRAP */
{
				ULONG srcreg = (opcode & 15);
{{      ULONG src = srcreg;
				Exception(src+32,0);
}}}
void op_4e50(void) /* LINK */
{
				ULONG srcreg = (opcode & 7);
{{      musashik_regs.a[7] -= 4;
{       CPTR olda = musashik_regs.a[7];
{       LONG src = musashik_regs.a[srcreg];
	put_long(olda,src);
	musashik_regs.a[srcreg] = (musashik_regs.a[7]);
{       WORD offs = nextiword();
        musashik_regs.a[7] += offs;
}}}}}}
void op_4e58(void) /* UNLK */
{
        ULONG srcreg = (opcode & 7);
{{      LONG src = musashik_regs.a[srcreg];
        musashik_regs.a[7] = src;
{       CPTR olda = musashik_regs.a[7];
        LONG old = get_long(olda);
{       musashik_regs.a[7] += 4;
        musashik_regs.a[srcreg] = (old);
}}}}}
void op_4e60(void) /* MVR2USP */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      LONG src = musashik_regs.a[srcreg];
        musashik_regs.usp = src;
}}}}
void op_4e68(void) /* MVUSP2R */
{
        ULONG srcreg = (opcode & 7);
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      musashik_regs.a[srcreg] = (musashik_regs.usp);
}}}}
void op_4e70(void) /* RESET */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{
}}}
void op_4e71(void) /* NOP */
{
{}}
void op_4e72(void) /* STOP */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      WORD src = nextiword();
        musashik_regs.sr = src;
        MakeFromSR();
        pending_interrupts |= MC_MUSASHI000_STOP;
        MC_MUSASHI000_ICount = 0;
}}}}
void op_4e73(void) /* RTE */
{
{if (!musashik_regs.s) { musashik_regs.pc -= 2; Exception(8,0); } else
{{      CPTR sra = musashik_regs.a[7];
        WORD sr = get_word(sra);
{       musashik_regs.a[7] += 2;
{       CPTR pca = musashik_regs.a[7];
        LONG pc = get_long(pca);
{       musashik_regs.a[7] += 4;
        musashik_regs.sr = sr; musashik_setpc(pc);
        MakeFromSR();
}}}}}}}
void op_4e74(void) /* RTD */
{
{{      CPTR pca = musashik_regs.a[7];
        LONG pc = get_long(pca);
{       musashik_regs.a[7] += 4;
{       WORD offs = nextiword();
        musashik_regs.a[7] += offs;
        musashik_setpc(pc);
}}}}}
void op_4e75(void) /* RTS */
{
{{      CPTR pca = musashik_regs.a[7];
        LONG pc = get_long(pca);
{       musashik_regs.a[7] += 4;
        musashik_setpc(pc);
}}}}
void op_4e76(void) /* TRAPV */
{
{       if(VFLG) Exception(7,musashik_getpc()-2);
}}
void op_4e77(void) /* RTR */
{
{       MakeSR();
{       CPTR sra = musashik_regs.a[7];
        WORD sr = get_word(sra);
{       musashik_regs.a[7] += 2;
{       CPTR pca = musashik_regs.a[7];
        LONG pc = get_long(pca);
{       musashik_regs.a[7] += 4;
        musashik_regs.sr &= 0xFF00; sr &= 0xFF;
        musashik_regs.sr |= sr; musashik_setpc(pc);
        MakeFromSR();
}}}}}}
void op_4e90(void) /* JSR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
{       musashik_regs.a[7] -= 4;
{       CPTR spa = musashik_regs.a[7];
        put_long(spa,musashik_getpc());
        musashik_setpc(srca);
}}}}}
void op_4ea8(void) /* JSR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{       musashik_regs.a[7] -= 4;
{       CPTR spa = musashik_regs.a[7];
        put_long(spa,musashik_getpc());
        musashik_setpc(srca);
}}}}}
void op_4eb0(void) /* JSR */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{       musashik_regs.a[7] -= 4;
{       CPTR spa = musashik_regs.a[7];
        put_long(spa,musashik_getpc());
        musashik_setpc(srca);
}}}}}
void op_4eb8(void) /* JSR */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
{       musashik_regs.a[7] -= 4;
{       CPTR spa = musashik_regs.a[7];
        put_long(spa,musashik_getpc());
        musashik_setpc(srca);
}}}}}
void op_4eb9(void) /* JSR */
{
{{      CPTR srca = nextilong();
{       musashik_regs.a[7] -= 4;
{       CPTR spa = musashik_regs.a[7];
        put_long(spa,musashik_getpc());
        musashik_setpc(srca);
}}}}}
void op_4eba(void) /* JSR */
{
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
{       musashik_regs.a[7] -= 4;
{       CPTR spa = musashik_regs.a[7];
        put_long(spa,musashik_getpc());
        musashik_setpc(srca);
}}}}}
void op_4ebb(void) /* JSR */
{
{{      CPTR srca = get_disp_ea(musashik_getpc());
{       musashik_regs.a[7] -= 4;
{       CPTR spa = musashik_regs.a[7];
        put_long(spa,musashik_getpc());
        musashik_setpc(srca);
}}}}}
void op_4ed0(void) /* JMP */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg];
        musashik_setpc(srca);
}}}
void op_4ee8(void) /* JMP */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
        musashik_setpc(srca);
}}}
void op_4ef0(void) /* JMP */
{
        ULONG srcreg = (opcode & 7);
{{      CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
        musashik_setpc(srca);
}}}
void op_4ef8(void) /* JMP */
{
{{      CPTR srca = (LONG)(WORD)nextiword();
        musashik_setpc(srca);
}}}
void op_4ef9(void) /* JMP */
{
{{      CPTR srca = nextilong();
        musashik_setpc(srca);
}}}
void op_4efa(void) /* JMP */
{
{{      CPTR srca = musashik_getpc();
        srca += (LONG)(WORD)nextiword();
        musashik_setpc(srca);
}}}
void op_4efb(void) /* JMP */
{
{{      CPTR srca = get_disp_ea(musashik_getpc());
        musashik_setpc(srca);
}}}
void op_5000(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5010(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5018(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_5020(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_5028(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5030(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_5038(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5039(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5040(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5048(void) /* ADDA */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_5050(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5058(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_5060(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_5068(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5070(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_5078(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5079(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5080(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5088(void) /* ADDA */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_5090(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_5098(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_50a0(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_50a8(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_50b0(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_50b8(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_50b9(void) /* ADD */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}

void op_50c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
	int val = cctrue(0) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}

void op_50c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(0))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}
void op_50d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	int val = cctrue(0) ? 0xff : 0;
	put_byte(srca,val);
}

void op_50d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(0) ? 0xff : 0;
	put_byte(srca,val);
}}

void op_50e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	int val = cctrue(0) ? 0xff : 0;
	put_byte(srca,val);
}}

void op_50e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(0) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_50f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(0) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_50f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(0) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_50f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(0) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5100(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}

void op_5110(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5118(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_5120(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_5128(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5130(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_5138(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5139(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5140(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5148(void) /* SUBA */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_5150(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5158(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_5160(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_5168(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5170(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_5178(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5179(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5180(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5188(void) /* SUBA */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_5190(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_5198(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_51a0(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_51a8(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_51b0(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG src = srcreg;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_51b8(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_51b9(void) /* SUB */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
{{	ULONG src = srcreg;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_51c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(1) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}

void op_51c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(1))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_51d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(1) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_51d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(1) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_51e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(1) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_51e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(1) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_51f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(1) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_51f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(1) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_51f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(1) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_52c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(2) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_52c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(2))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_52d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	int val = cctrue(2) ? 0xff : 0;
	put_byte(srca,val);
}

void op_52d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(2) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_52e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(2) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_52e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(2) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_52f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(2) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_52f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(2) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_52f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(2) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_53c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(3) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}

void op_53c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(3))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_53d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(3) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_53d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(3) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_53e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(3) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_53e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(3) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_53f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(3) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_53f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(3) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_53f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(3) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_54c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(4) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_54c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(4))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_54d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(4) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_54d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(4) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_54e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(4) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_54e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(4) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_54f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(4) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_54f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(4) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_54f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(4) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_55c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(5) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_55c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(5))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_55d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(5) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_55d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(5) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_55e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(5) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_55e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(5) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_55f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(5) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_55f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(5) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_55f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(5) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_56c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(6) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_56c8(void) /* DBcc */
{
	ULONG srcreg = (opcode & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD offs = nextiword();
	if (!cctrue(6)) {
	if (src--) { musashik_regs.pc += (LONG)offs - 2; /*change_pc24(musashik_regs.pc&0xffffff);*/ }	/* ASG 971108 */
	musashik_regs.d[srcreg].W.l = src;
	}
}}}}
void op_56d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(6) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_56d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(6) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_56e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(6) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_56e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(6) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_56f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(6) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_56f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(6) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_56f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(6) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_57c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(7) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_57c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(7))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_57d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
	CPTR srca = musashik_regs.a[srcreg];
	int val = cctrue(7) ? 0xff : 0;
	put_byte(srca,val);
}

void op_57d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(7) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_57e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(7) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_57e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(7) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_57f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(7) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_57f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(7) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_57f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(7) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_58c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(8) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}

void op_58c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(8))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_58d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(8) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_58d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(8) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_58e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(8) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_58e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(8) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_58f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(8) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_58f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(8) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_58f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(8) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_59c0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(9) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_59c8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(9))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_59d0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(9) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_59d8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(9) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_59e0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(9) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_59e8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(9) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_59f0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(9) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_59f8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(9) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_59f9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(9) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ac0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(10) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_5ac8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(10))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_5ad0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(10) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ad8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(10) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5ae0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(10) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5ae8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(10) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5af0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(10) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5af8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(10) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5af9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(10) ? 0xff : 0;
	put_byte(srca,val);
}}}}

void op_5bc0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
	int val = cctrue(11) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}

void op_5bc8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(11))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_5bd0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(11) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5bd8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(11) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5be0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(11) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5be8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(11) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5bf0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(11) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5bf8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(11) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5bf9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(11) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5cc0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(12) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_5cc8(void) /* DBcc */
{
	WORD offs = nextiword();
	if (!cctrue(12))
		{
		ULONG srcreg = (opcode & 7);
		WORD src = musashik_regs.d[srcreg].W.l;
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_5cd0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(12) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5cd8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(12) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5ce0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(12) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5ce8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(12) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5cf0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(12) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5cf8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(12) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5cf9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(12) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5dc0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(13) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}

void op_5dc8(void) /* DBcc */
{
	ULONG srcreg = (opcode & 7);
	if (!cctrue(13))
		{
		WORD src = musashik_regs.d[srcreg].W.l;
		WORD offs = nextiword();
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_5dd0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(13) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5dd8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(13) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5de0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(13) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5de8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(13) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5df0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(13) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5df8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(13) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5df9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(13) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ec0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(14) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_5ec8(void) /* DBcc */
{
	ULONG srcreg = (opcode & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD offs = nextiword();
	if (!cctrue(14)) {
	if (src--) { musashik_regs.pc += (LONG)offs - 2; /*change_pc24(musashik_regs.pc&0xffffff);*/ }	/* ASG 971108 */
	musashik_regs.d[srcreg].W.l = src;
	}
}}}}
void op_5ed0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(14) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ed8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(14) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5ee0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(14) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5ee8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(14) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ef0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(14) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ef8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(14) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ef9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(14) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5fc0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{{	int val = cctrue(15) ? 0xff : 0;
	musashik_regs.d[srcreg].B.l = val;
}}}}
void op_5fc8(void) /* DBcc */
{
	ULONG srcreg = (opcode & 7);
	if (!cctrue(15))
		{
		WORD src = musashik_regs.d[srcreg].W.l;
		WORD offs = nextiword();
		if (src--)
			{
			musashik_regs.pc += (LONG)offs - 2;
			}
		musashik_regs.d[srcreg].W.l = src;
		}
}

void op_5fd0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(15) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5fd8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg];
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	int val = cctrue(15) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5fe0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
{	int val = cctrue(15) ? 0xff : 0;
	put_byte(srca,val);
}}}}}
void op_5fe8(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
{	int val = cctrue(15) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ff0(void) /* Scc */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	int val = cctrue(15) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ff8(void) /* Scc */
{
{{	CPTR srca = (LONG)(WORD)nextiword();
{	int val = cctrue(15) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_5ff9(void) /* Scc */
{
{{	CPTR srca = nextilong();
{	int val = cctrue(15) ? 0xff : 0;
	put_byte(srca,val);
}}}}
void op_6000(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	WORD src = nextiword();
	if (cctrue(0))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
		}
}

void op_6001(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
	ULONG oldpc = musashik_regs.pc;
	ULONG src = srcreg;
	if (cctrue(0))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		}
}
void op_60ff(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	if (cctrue(0))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		change_pc24 (musashik_regs.pc&0xffffff);
		}
}

void op_6100(void) /* BSR */
{
	ULONG oldpc = musashik_regs.pc;
	WORD src = nextiword();
	musashik_regs.a[7] -= 4;
{	CPTR spa = musashik_regs.a[7];
	put_long(spa,musashik_getpc());
	musashik_regs.pc = oldpc + (LONG)src;
	change_pc24 (musashik_regs.pc&0xffffff);
}}

void op_6101(void) /* BSR */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
	ULONG oldpc = musashik_regs.pc;
	ULONG src = srcreg;
	musashik_regs.a[7] -= 4;
{	CPTR spa = musashik_regs.a[7];
	put_long(spa,musashik_getpc());
	musashik_regs.pc = oldpc + (LONG)src;
}}

void op_61ff(void) /* BSR */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	musashik_regs.a[7] -= 4;
{	CPTR spa = musashik_regs.a[7];
	put_long(spa,musashik_getpc());
	musashik_regs.pc = oldpc + (LONG)src;
	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
}}

void op_6200(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	WORD src = nextiword();
	if (cctrue(2))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
		}
}

void op_6201(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
	ULONG oldpc = musashik_regs.pc;
	ULONG src = srcreg;
	if (cctrue(2))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		}
}

void op_62ff(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	if (cctrue(2))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		change_pc24 (musashik_regs.pc&0xffffff);
		}
}

void op_6300(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	WORD src = nextiword();
	if (cctrue(3))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		change_pc24 (musashik_regs.pc&0xffffff);
		}
}

void op_6301(void) /* Bcc */
{
	if (cctrue(3))
		{
		musashik_regs.pc += (LONG)(BYTE)(opcode&255);
		}
}

void op_63ff(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	if (cctrue(3))
		{
		musashik_regs.pc = oldpc + (LONG)src;
		change_pc24 (musashik_regs.pc&0xffffff);
		}
}

void op_6400(void) /* Bcc */
{
	WORD src = nextiword();
	if (cctrue(4))
		{
		musashik_regs.pc += (LONG)src-2;
		change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
		}
}

void op_6401(void) /* Bcc */
{
	if (cctrue(4))
		{
		musashik_regs.pc += (LONG)(BYTE)(opcode&255);
		}
}

void op_64ff(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	if (cctrue(4)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}

void op_6500(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	WORD src = nextiword();
	if (cctrue(5)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}

void op_6501(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
	ULONG oldpc = musashik_regs.pc;
	ULONG src = srcreg;
	if (cctrue(5)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}

void op_65ff(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	if (cctrue(5)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}

void op_6600(void) /* Bcc */
{
	WORD src = nextiword();
	if (cctrue(6)) {
		musashik_regs.pc += (LONG)src-2;
		change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
		}
}

void op_6601(void) /* Bcc */
{
	if (cctrue(6))
		{
		musashik_regs.pc += (LONG)(BYTE)(opcode&255);
		}
}

void op_66ff(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	if (cctrue(6)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}

void op_6700(void) /* Bcc */
{
	WORD src = nextiword();
	if (cctrue(7))
		{
		musashik_regs.pc += (LONG)src-2;
		change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
		}
}

void op_6701(void) /* Bcc */
{
	if (cctrue(7))
		{
		musashik_regs.pc += (LONG)(BYTE)(opcode & 255);
		}
}

void op_67ff(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	LONG src = nextilong();
	if (cctrue(7)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}

void op_6800(void) /* Bcc */
{
	ULONG oldpc = musashik_regs.pc;
	WORD src = nextiword();
	if (cctrue(8)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}

void op_6801(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(8)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_68ff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(8)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6900(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	WORD src = nextiword();
	if (cctrue(9)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6901(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(9)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_69ff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(9)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6a00(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	WORD src = nextiword();
	if (cctrue(10)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6a01(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(10)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_6aff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(10)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6b00(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	WORD src = nextiword();
	if (cctrue(11)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6b01(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(11)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_6bff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(11)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6c00(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	WORD src = nextiword();
	if (cctrue(12)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6c01(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(12)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_6cff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(12)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6d00(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	WORD src = nextiword();
	if (cctrue(13)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6d01(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(13)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_6dff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(13)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6e00(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	WORD src = nextiword();
	if (cctrue(14)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6e01(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(14)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_6eff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(14)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6f00(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	WORD src = nextiword();
	if (cctrue(15)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_6f01(void) /* Bcc */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
{	ULONG oldpc = musashik_regs.pc;
{	ULONG src = srcreg;
	if (cctrue(15)) {
	musashik_regs.pc = oldpc + (LONG)src;	/*change_pc24(musashik_regs.pc&0xffffff);*/	/* ASG 971108 */
	}
}}}
void op_6fff(void) /* Bcc */
{
{	ULONG oldpc = musashik_regs.pc;
{	LONG src = nextilong();
	if (cctrue(15)) {
	musashik_regs.pc = oldpc + (LONG)src;	change_pc24 (musashik_regs.pc&0xffffff);	/* ASG 971108 */
	}
}}}
void op_7000(void) /* MOVE */
{
	ULONG srcreg = (LONG)(BYTE)(opcode & 255);
	ULONG dstreg = (opcode >> 9) & 7;
{{	ULONG src = srcreg;
{	CLEARVC;
	ZFLG = ((LONG)(src)) == 0;
	NFLG = ((LONG)(src)) < 0;
	musashik_regs.d[dstreg].D = (src);
}}}}

/* BFV 061298 - DIVU/DIVS opcodes are modified to decrement the icount */

void op_8000(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}

void op_8010(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8018(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8020(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8028(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8030(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8038(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8039(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_803a(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_803b(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_803c(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = nextiword();
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src |= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8040(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;

{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8050(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8058(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8060(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8068(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8070(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8078(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8079(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_807a(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_807b(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_807c(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src |= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8080(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8090(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8098(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[srcreg] += 4;
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_80a0(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_80a8(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_80b0(void) /* OR */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_80b8(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_80b9(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_80ba(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_80bb(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_80bc(void) /* OR */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.d[dstreg].D;
	src |= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_80c0(void) /* DIVU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	WORD src = musashik_regs.d[srcreg].W.l;
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 38;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 133;
	}
}}}}
void op_80d0(void) /* DIVU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 42;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 137;
	}
}}}}
void op_80d8(void) /* DIVU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 42;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 137;
	}
}}}}}
void op_80e0(void) /* DIVU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 44;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 139;
	}
}}}}}
void op_80e8(void) /* DIVU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 46;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 141;
	}
}}}}
void op_80f0(void) /* DIVU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 48;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 143;
	}
}}}}}
void op_80f8(void) /* DIVU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 46;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 141;
	}
}}}}
void op_80f9(void) /* DIVU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 50;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 145;
	}
}}}}
void op_80fa(void) /* DIVU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 46;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 141;
	}
}}}}}
void op_80fb(void) /* DIVU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 48;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 143;
	}
}}}}}
void op_80fc(void) /* DIVU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	WORD src = nextiword();
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 42;
	Exception(5,oldpc-2);} else {
	ULONG newv = (ULONG)dst / (ULONG)(UWORD)src;
	ULONG rem = (ULONG)dst % (ULONG)(UWORD)src;
	if (newv > 0xffff) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 137;
	}
}}}}
void op_8100(void) /* SBCD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{	UWORD newv_lo = (dst & 0xF) - (src & 0xF) - (musashik_regs.x ? 1 : 0);
	UWORD newv_hi = (dst & 0xF0) - (src & 0xF0);
	UWORD newv;
	if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
	newv = newv_hi + (newv_lo & 0xF);	CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
	if (CFLG) newv -= 0x60;
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	musashik_regs.d[dstreg].B.l = newv;
}}}}}}
void op_8108(void) /* SBCD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	UWORD newv_lo = (dst & 0xF) - (src & 0xF) - (musashik_regs.x ? 1 : 0);
	UWORD newv_hi = (dst & 0xF0) - (src & 0xF0);
	UWORD newv;
	if (newv_lo > 9) { newv_lo-=6; newv_hi-=0x10; }
	newv = newv_hi + (newv_lo & 0xF);	CFLG = musashik_regs.x = (newv_hi & 0x1F0) > 0x90;
	if (CFLG) newv -= 0x60;
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	put_byte(dsta,newv);
}}}}}}}}
void op_8110(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8118(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8120(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8128(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8130(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8138(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8139(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src |= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8150(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8158(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8160(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8168(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8170(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_8178(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8179(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	src |= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8190(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_8198(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_81a0(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_81a8(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src |= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_81b0(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
	src |= dst;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
	put_long(dsta,src);
}}}}}
void op_81b8(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src |= dst;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
	put_long(dsta,src);
}}}}
void op_81b9(void) /* OR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	src |= dst;
	CLEARVC;
	ZFLG = ((LONG)(src)) == 0;
	NFLG = ((LONG)(src)) < 0;
	put_long(dsta,src);
}}}}
void op_81c0(void) /* DIVS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	WORD src = musashik_regs.d[srcreg].W.l;
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 38;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 150;
	}
}}}}
void op_81d0(void) /* DIVS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 42;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 154;
	}
}}}}
void op_81d8(void) /* DIVS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 42;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 154;
	}
}}}}}
void op_81e0(void) /* DIVS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 44;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 156;
	}
}}}}}
void op_81e8(void) /* DIVS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 46;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 158;
	}
}}}}
void op_81f0(void) /* DIVS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 48;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 160;
	}
}}}}}
void op_81f8(void) /* DIVS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 46;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 158;
	}
}}}}
void op_81f9(void) /* DIVS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 50;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 162;
	}
}}}}
void op_81fa(void) /* DIVS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 46;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 158;
	}
}}}}}
void op_81fb(void) /* DIVS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 48;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 160;
	}
}}}}}
void op_81fc(void) /* DIVS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{	CPTR oldpc = musashik_getpc();
{	WORD src = nextiword();
{	LONG dst = musashik_regs.d[dstreg].D;
	if(src == 0){	MC_MUSASHI000_ICount -= 42;
	Exception(5,oldpc-2);} else {
	LONG newv = (LONG)dst / (LONG)(WORD)src;
	UWORD rem = (LONG)dst % (LONG)(WORD)src;
	if ((newv & 0xffff8000) != 0 && (newv & 0xffff8000) != 0xffff8000) { VFLG = NFLG = 1; CFLG = 0; } else
	{
	if (((WORD)rem < 0) != ((LONG)dst < 0)) rem = -rem;
	CLEARVC;
	ZFLG = ((WORD)(newv)) == 0;
	NFLG = ((WORD)(newv)) < 0;
	newv = (newv & 0xffff) | ((ULONG)rem << 16);
	musashik_regs.d[dstreg].D = (newv);
	}
	MC_MUSASHI000_ICount -= 154;
	}
}}}}
void op_9000(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9010(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9018(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9020(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9028(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9030(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9038(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9039(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_903a(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_903b(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_903c(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = nextiword();
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9040(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9048(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9050(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9058(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9060(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9068(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9070(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9078(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9079(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_907a(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_907b(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_907c(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9080(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9088(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9090(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9098(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[srcreg] += 4;
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_90a0(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_90a8(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_90b0(void) /* SUB */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_90b8(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_90b9(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_90ba(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_90bb(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_90bc(void) /* SUB */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_90c0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_90c8(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_90d0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_90d8(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_90e0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_90e8(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_90f0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_90f8(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_90f9(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_90fa(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_90fb(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_90fc(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_9100(void) /* SUBX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{	ULONG newv = dst - src - (musashik_regs.x ? 1 : 0);
	musashik_regs.d[dstreg].B.l = newv;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (!flgs && flgo && !flgn) || (flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && !flgo) || (flgn && (!flgo || flgs));
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
}}}}}}
void op_9108(void) /* SUBX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	ULONG newv = dst - src - (musashik_regs.x ? 1 : 0);
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (!flgs && flgo && !flgn) || (flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && !flgo) || (flgn && (!flgo || flgs));
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
}}}}}}}}
void op_9110(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9118(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9120(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9128(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9130(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9138(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9139(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	put_byte(dsta,newv);
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	ZFLG = ((BYTE)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9140(void) /* SUBX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	ULONG newv = dst - src - (musashik_regs.x ? 1 : 0);
	musashik_regs.d[dstreg].W.l = newv;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (!flgs && flgo && !flgn) || (flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && !flgo) || (flgn && (!flgo || flgs));
	if (((WORD)(newv)) != 0) ZFLG = 0;
	NFLG = ((WORD)(newv)) < 0;
}}}}}}
void op_9148(void) /* SUBX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	ULONG newv = dst - src - (musashik_regs.x ? 1 : 0);
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (!flgs && flgo && !flgn) || (flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && !flgo) || (flgn && (!flgo || flgs));
	if (((WORD)(newv)) != 0) ZFLG = 0;
	NFLG = ((WORD)(newv)) < 0;
}}}}}}}}
void op_9150(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9158(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9160(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9168(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9170(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_9178(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9179(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9180(void) /* SUBX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
{	ULONG newv = dst - src - (musashik_regs.x ? 1 : 0);
	musashik_regs.d[dstreg].D = (newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (!flgs && flgo && !flgn) || (flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && !flgo) || (flgn && (!flgo || flgs));
	if (((LONG)(newv)) != 0) ZFLG = 0;
	NFLG = ((LONG)(newv)) < 0;
}}}}}}
void op_9188(void) /* SUBX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	ULONG newv = dst - src - (musashik_regs.x ? 1 : 0);
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (!flgs && flgo && !flgn) || (flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && !flgo) || (flgn && (!flgo || flgs));
	if (((LONG)(newv)) != 0) ZFLG = 0;
	NFLG = ((LONG)(newv)) < 0;
}}}}}}}}
void op_9190(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_9198(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_91a0(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_91a8(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_91b0(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_91b8(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_91b9(void) /* SUB */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	put_long(dsta,newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_91c0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_91c8(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_91d0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_91d8(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	musashik_regs.a[srcreg] += 4;
{	LONG src = get_long(srca);
	LONG dst = musashik_regs.a[dstreg];
	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}

void op_91e0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_91e8(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_91f0(void) /* SUBA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_91f8(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_91f9(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_91fa(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_91fb(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_91fc(void) /* SUBA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst - src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_b000(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b010(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b018(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_b020(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_b028(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b030(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b038(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b039(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b03a(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}}

void op_b03b(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	BYTE src = get_byte(srca);
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b03c(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	BYTE src = nextiword();
	BYTE dst = musashik_regs.d[dstreg].B.l;
	ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
}}

void op_b040(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	WORD src = musashik_regs.d[srcreg].W.l;
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b048(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	WORD src = musashik_regs.a[srcreg];
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b050(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b058(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
	musashik_regs.a[srcreg] += 2;
{	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_b060(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_b068(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b070(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b078(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b079(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b07a(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}}

void op_b07b(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	WORD src = get_word(srca);
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b07c(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	WORD src = nextiword();
	WORD dst = musashik_regs.d[dstreg].W.l;
	ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
}}

void op_b080(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.d[srcreg].D;
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b088(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = musashik_regs.a[srcreg];
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b090(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b098(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	musashik_regs.a[srcreg] += 4;
{	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_b0a0(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_b0a8(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0b0(void) /* CMP */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0b8(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0b9(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = nextilong();
	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0ba(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}}

void op_b0bb(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	CPTR srca = get_disp_ea(musashik_getpc());
	LONG src = get_long(srca);
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0bc(void) /* CMP */
{
	ULONG dstreg = (opcode >> 9) & 7;
	LONG src = nextilong();
	LONG dst = musashik_regs.d[dstreg].D;
	ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	NFLG = flgn != 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
}}

void op_b0c0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0c8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0d0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0d8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0e0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0e8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0f0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0f8(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0f9(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b0fa(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0fb(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b0fc(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b100(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src ^= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b108(void) /* CMPM */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{{ULONG newv = ((BYTE)(dst)) - ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UBYTE)(src)) > ((UBYTE)(dst));
	NFLG = flgn != 0;
}}}}}}}}}
void op_b110(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b118(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b120(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b128(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b130(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b138(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b139(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src ^= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b140(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src ^= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b148(void) /* CMPM */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
{{ULONG newv = ((WORD)(dst)) - ((WORD)(src));
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((UWORD)(src)) > ((UWORD)(dst));
	NFLG = flgn != 0;
}}}}}}}}}
void op_b150(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b158(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b160(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b168(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b170(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b178(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b179(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	src ^= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b180(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
	src ^= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b188(void) /* CMPM */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[srcreg] += 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}}
void op_b190(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b198(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b1a0(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b1a8(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b1b0(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_b1b8(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b1b9(void) /* EOR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	src ^= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_b1c0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1c8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1d0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1d8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[srcreg] += 4;
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1e0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1e8(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1f0(void) /* CMPA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1f8(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1f9(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_b1fa(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1fb(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}}
void op_b1fc(void) /* CMPA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.a[dstreg];
{{ULONG newv = ((LONG)(dst)) - ((LONG)(src));
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	CFLG = ((ULONG)(src)) > ((ULONG)(dst));
	NFLG = flgn != 0;
}}}}}}}
void op_c000(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c010(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c018(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c020(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c028(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c030(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c038(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c039(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c03a(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c03b(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c03c(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = nextiword();
{	BYTE dst = musashik_regs.d[dstreg].B.l;
	src &= dst;
	musashik_regs.d[dstreg].B.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c040(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c050(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c058(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c060(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c068(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c070(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c078(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c079(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c07a(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c07b(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c07c(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
	src &= dst;
	musashik_regs.d[dstreg].W.l = src;
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c080(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c090(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c098(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[srcreg] += 4;
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0a0(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0a8(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c0b0(void) /* AND */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0b8(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c0b9(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c0ba(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0bb(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0bc(void) /* AND */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.d[dstreg].D;
	src &= dst;
	musashik_regs.d[dstreg].D = (src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c0c0(void) /* MULU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0d0(void) /* MULU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	CLEARFLGS;
	musashik_regs.d[dstreg].D = (newv);
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0d8(void) /* MULU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c0e0(void) /* MULU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c0e8(void) /* MULU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0f0(void) /* MULU */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c0f8(void) /* MULU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0f9(void) /* MULU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c0fa(void) /* MULU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c0fb(void) /* MULU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c0fc(void) /* MULU */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (ULONG)(UWORD)dst * (ULONG)(UWORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c100(void) /* ABCD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{	UWORD newv_lo = (src & 0xF) + (dst & 0xF) + (musashik_regs.x ? 1 : 0);
	UWORD newv_hi = (src & 0xF0) + (dst & 0xF0);
	UWORD newv;
	if (newv_lo > 9) { newv_lo +=6; }
	newv = newv_hi + newv_lo;
	CFLG = musashik_regs.x = (newv & 0x1F0) > 0x90;
	if (CFLG) newv += 0x60;
	musashik_regs.d[dstreg].B.l = newv;
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
}}}}}}
void op_c108(void) /* ABCD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	UWORD newv_lo = (src & 0xF) + (dst & 0xF) + (musashik_regs.x ? 1 : 0);
	UWORD newv_hi = (src & 0xF0) + (dst & 0xF0);
	UWORD newv;
	if (newv_lo > 9) { newv_lo +=6; }
	newv = newv_hi + newv_lo;
	CFLG = musashik_regs.x = (newv & 0x1F0) > 0x90;
	if (CFLG) newv += 0x60;
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs != flgo) && (flgn != flgo);
	put_byte(dsta,newv);
}}}}}}}}
void op_c110(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c118(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c120(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c128(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c130(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c138(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c139(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
	src &= dst;
	put_byte(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c140(void) /* EXG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
	LONG src = musashik_regs.d[srcreg].D;
	musashik_regs.d[srcreg].D = musashik_regs.d[dstreg].D;
	musashik_regs.d[dstreg].D = src;
}

void op_c148(void) /* EXG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
	LONG src = musashik_regs.a[srcreg];
	musashik_regs.a[srcreg] = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] = src;
}

void op_c150(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c158(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c160(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c168(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c170(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c178(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c179(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
	src &= dst;
	put_word(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c188(void) /* EXG */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
	LONG src = musashik_regs.d[srcreg].D;
	musashik_regs.d[srcreg].D = musashik_regs.a[dstreg];
	musashik_regs.a[dstreg] = (src);
}

void op_c190(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c198(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1a0(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1a8(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c1b0(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1b8(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c1b9(void) /* AND */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
	src &= dst;
	put_long(dsta,src);
	CLEARFLGS;
	if ( src <= 0 )
		{
		if ( src ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}
void op_c1c0(void) /* MULS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1d0(void) /* MULS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1d8(void) /* MULS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c1e0(void) /* MULS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c1e8(void) /* MULS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1f0(void) /* MULS */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c1f8(void) /* MULS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1f9(void) /* MULS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_c1fa(void) /* MULS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c1fb(void) /* MULS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}}
void op_c1fc(void) /* MULS */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	LONG newv = (LONG)(WORD)dst * (LONG)(WORD)src;
	musashik_regs.d[dstreg].D = (newv);
	CLEARFLGS;
	if ( newv <= 0 )
		{
		if ( newv ==  0 )
			{
			ZFLG = ZTRUE;
			}
		else
			{
			NFLG = NTRUE;
			}
		}
}}}}}
void op_d000(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
	musashik_regs.d[dstreg].B.l = newv;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d010(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d018(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	musashik_regs.a[srcreg] += areg_byteinc[srcreg];
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
	musashik_regs.d[dstreg].B.l = newv;
}}}}}}}}
void op_d020(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
	musashik_regs.d[dstreg].B.l = newv;
}}}}}}}}
void op_d028(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d030(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d038(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d039(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d03a(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d03b(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	BYTE src = get_byte(srca);
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d03c(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = nextiword();
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	musashik_regs.d[dstreg].B.l = newv;
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d040(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d048(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d050(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d058(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d060(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d068(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d070(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d078(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d079(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d07a(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d07b(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d07c(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	WORD dst = musashik_regs.d[dstreg].W.l;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	musashik_regs.d[dstreg].W.l = newv;
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d080(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d088(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d090(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d098(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[srcreg] += 4;
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0a0(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0a8(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0b0(void) /* ADD */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0b8(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0b9(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0ba(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0bb(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d0bc(void) /* ADD */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.d[dstreg].D;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	musashik_regs.d[dstreg].D = (newv);
	ZFLG = ((LONG)(newv)) == 0;
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d0c0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d0c8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d0d0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d0d8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[srcreg] += 2;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d0e0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d0e8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d0f0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d0f8(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d0f9(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d0fa(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d0fb(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	WORD src = get_word(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d0fc(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = nextiword();
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d100(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	BYTE dst = musashik_regs.d[dstreg].B.l;
{	ULONG newv = dst + src + (musashik_regs.x ? 1 : 0);
	musashik_regs.d[dstreg].B.l = newv;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
}}}}}}

void op_d108(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
	musashik_regs.a[srcreg] -= areg_byteinc[srcreg];
{	CPTR srca = musashik_regs.a[srcreg];
	BYTE src = get_byte(srca);
	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	ULONG newv = dst + src + (musashik_regs.x ? 1 : 0);
	put_byte(dsta,newv);
	if (((BYTE)(newv)) != 0) ZFLG = 0;
	NFLG = ((BYTE)(newv)) < 0;
{	int flgn = ((BYTE)(newv)) < 0;
	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
}}}}

void op_d110(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d118(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
	BYTE src = musashik_regs.d[srcreg].B.l;
	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
	musashik_regs.a[dstreg] += areg_byteinc[dstreg];
{	ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgn = ((BYTE)(newv)) < 0;
	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}

void op_d120(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	musashik_regs.a[dstreg] -= areg_byteinc[dstreg];
{	CPTR dsta = musashik_regs.a[dstreg];
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d128(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d130(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d138(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d139(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	BYTE src = musashik_regs.d[srcreg].B.l;
{	CPTR dsta = nextilong();
	BYTE dst = get_byte(dsta);
{{ULONG newv = ((BYTE)(dst)) + ((BYTE)(src));
	put_byte(dsta,newv);
	ZFLG = ((BYTE)(newv)) == 0;
{	int flgs = ((BYTE)(src)) < 0;
	int flgo = ((BYTE)(dst)) < 0;
	int flgn = ((BYTE)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UBYTE)(~dst)) < ((UBYTE)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d140(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	WORD dst = musashik_regs.d[dstreg].W.l;
{	ULONG newv = dst + src + (musashik_regs.x ? 1 : 0);
	musashik_regs.d[dstreg].W.l = newv;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((WORD)(newv)) != 0) ZFLG = 0;
	NFLG = ((WORD)(newv)) < 0;
}}}}}}
void op_d148(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR srca = musashik_regs.a[srcreg];
	WORD src = get_word(srca);
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	ULONG newv = dst + src + (musashik_regs.x ? 1 : 0);
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((WORD)(newv)) != 0) ZFLG = 0;
	NFLG = ((WORD)(newv)) < 0;
}}}}}}}}
void op_d150(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d158(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{	musashik_regs.a[dstreg] += 2;
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	ZFLG = ((WORD)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d160(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	musashik_regs.a[dstreg] -= 2;
{	CPTR dsta = musashik_regs.a[dstreg];
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d168(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d170(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d178(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = (LONG)(WORD)nextiword();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d179(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	WORD src = musashik_regs.d[srcreg].W.l;
{	CPTR dsta = nextilong();
	WORD dst = get_word(dsta);
{{ULONG newv = ((WORD)(dst)) + ((WORD)(src));
	put_word(dsta,newv);
	ZFLG = ((WORD)(newv)) == 0;
{	int flgs = ((WORD)(src)) < 0;
	int flgo = ((WORD)(dst)) < 0;
	int flgn = ((WORD)(newv)) < 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((UWORD)(~dst)) < ((UWORD)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d180(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.d[dstreg].D;
{	ULONG newv = dst + src + (musashik_regs.x ? 1 : 0);
	musashik_regs.d[dstreg].D = (newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((LONG)(newv)) != 0) ZFLG = 0;
	NFLG = ((LONG)(newv)) < 0;
}}}}}}
void op_d188(void) /* ADDX */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	ULONG newv = dst + src + (musashik_regs.x ? 1 : 0);
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	VFLG = (flgs && flgo && !flgn) || (!flgs && !flgo && flgn);
	musashik_regs.x = CFLG = (flgs && flgo) || (!flgn && (flgo || flgs));
	if (((LONG)(newv)) != 0) ZFLG = 0;
	NFLG = ((LONG)(newv)) < 0;
}}}}}}}}
void op_d190(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d198(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{	musashik_regs.a[dstreg] += 4;
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d1a0(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	musashik_regs.a[dstreg] -= 4;
{	CPTR dsta = musashik_regs.a[dstreg];
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d1a8(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = musashik_regs.a[dstreg] + (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d1b0(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = get_disp_ea(musashik_regs.a[dstreg]);
{	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}}
void op_d1b8(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = (LONG)(WORD)nextiword();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d1b9(void) /* ADD */
{
	ULONG srcreg = ((opcode >> 9) & 7);
{{	LONG src = musashik_regs.d[srcreg].D;
{	CPTR dsta = nextilong();
	LONG dst = get_long(dsta);
{{ULONG newv = ((LONG)(dst)) + ((LONG)(src));
	put_long(dsta,newv);
{	int flgs = ((LONG)(src)) < 0;
	int flgo = ((LONG)(dst)) < 0;
	int flgn = ((LONG)(newv)) < 0;
	ZFLG = ((LONG)(newv)) == 0;
	VFLG = (flgs == flgo) && (flgn != flgo);
	CFLG = musashik_regs.x = ((ULONG)(~dst)) < ((ULONG)(src));
	NFLG = flgn != 0;
}}}}}}}
void op_d1c0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.d[srcreg].D;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d1c8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = musashik_regs.a[srcreg];
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d1d0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d1d8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	musashik_regs.a[srcreg] += 4;
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d1e0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	musashik_regs.a[srcreg] -= 4;
{	CPTR srca = musashik_regs.a[srcreg];
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d1e8(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d1f0(void) /* ADDA */
{
	ULONG srcreg = (opcode & 7);
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_regs.a[srcreg]);
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d1f8(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = (LONG)(WORD)nextiword();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d1f9(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = nextilong();
	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_d1fa(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = musashik_getpc();
	srca += (LONG)(WORD)nextiword();
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d1fb(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	CPTR srca = get_disp_ea(musashik_getpc());
{	LONG src = get_long(srca);
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}}
void op_d1fc(void) /* ADDA */
{
	ULONG dstreg = (opcode >> 9) & 7;
{{	LONG src = nextilong();
{	LONG dst = musashik_regs.a[dstreg];
{	ULONG newv = dst + src;
	musashik_regs.a[dstreg] = (newv);
}}}}}
void op_e000(void) /* ASR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	ULONG sign = cmask & val;
	cnt &= 63;
	CLEARFLGS;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		val = sign ? 0xff : 0;
		CFLG=musashik_regs.x= sign ? 1 : 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
		if (sign) val |= 0xff << (8 - cnt);
	}}
	musashik_regs.d[dstreg].B.l = val;
	NFLG = sign != 0;
	ZFLG = val == 0;
}}}}}
void op_e008(void) /* LSR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		CFLG=musashik_regs.x = cnt == 8 ? (val & cmask ? 1 : 0) : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
	}}
	musashik_regs.d[dstreg].B.l = val;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
}}}}}
void op_e010(void) /* ROXR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&1; val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry;
	}
	musashik_regs.d[dstreg].B.l = val;
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
}}}}}
void op_e018(void) /* ROR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {	ULONG carry;
	for(;cnt;--cnt){
	carry=val&1; val = val >> 1;
	if(carry) val |= cmask;
	}
	CFLG = carry;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e020(void) /* ASR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	ULONG sign = cmask & val;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		val = sign ? 0xff : 0;
		CFLG=musashik_regs.x= sign ? 1 : 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
		if (sign) val |= 0xff << (8 - cnt);
	}}
	musashik_regs.d[dstreg].B.l = val;
	NFLG = sign != 0;
	ZFLG = val == 0;
}}}}}
void op_e028(void) /* LSR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		CFLG=musashik_regs.x = cnt == 8 ? (val & cmask ? 1 : 0) : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
	}}
	musashik_regs.d[dstreg].B.l = val;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
}}}}}
void op_e030(void) /* ROXR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&1; val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry;
	}
	musashik_regs.d[dstreg].B.l = val;
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
}}}}}
void op_e038(void) /* ROR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {	ULONG carry;
	for(;cnt;--cnt){
	carry=val&1; val = val >> 1;
	if(carry) val |= cmask;
	}
	CFLG = carry;
}
	musashik_regs.d[dstreg].B.l = val;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
}}}}}
void op_e040(void) /* ASR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	cnt &= 63;
	CLEARFLGS;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		val = sign ? 0xffff : 0;
		CFLG=musashik_regs.x= sign ? 1 : 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
		if (sign) val |= 0xffff << (16 - cnt);
	}}
	musashik_regs.d[dstreg].W.l = val;
	NFLG = sign != 0;
	ZFLG = val == 0;
}}}}}
void op_e048(void) /* LSR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	CLEARFLGS;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		CFLG=musashik_regs.x = cnt == 16 ? (val & cmask ? 1 : 0) : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
	}}
	musashik_regs.d[dstreg].W.l = val;
	NFLG = (val & cmask) != 0; ZFLG = val == 0;
}}}}}

void op_e050(void) /* ROXR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&1; val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e058(void) /* ROR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {	ULONG carry;
	for(;cnt;--cnt){
	carry=val&1; val = val >> 1;
	if(carry) val |= cmask;
	}
	CFLG = carry;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e060(void) /* ASR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		val = sign ? 0xffff : 0;
		CFLG=musashik_regs.x= sign ? 1 : 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
		if (sign) val |= 0xffff << (16 - cnt);
	}}
	NFLG = sign != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e068(void) /* LSR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		CFLG=musashik_regs.x = cnt == 16 ? (val & cmask ? 1 : 0) : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e070(void) /* ROXR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&1; val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e078(void) /* ROR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {	ULONG carry;
	for(;cnt;--cnt){
	carry=val&1; val = val >> 1;
	if(carry) val |= cmask;
	}
	CFLG = carry;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e080(void) /* ASR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	ULONG sign = cmask & val;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		val = sign ? 0xffffffff : 0;
		CFLG=musashik_regs.x= sign ? 1 : 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
		if (sign) val |= 0xffffffff << (32 - cnt);
	}}
	NFLG = sign != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e088(void) /* LSR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		CFLG=musashik_regs.x = cnt == 32 ? (val & cmask ? 1 : 0) : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e090(void) /* ROXR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&1; val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e098(void) /* ROR */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {	ULONG carry;
	for(;cnt;--cnt){
	carry=val&1; val = val >> 1;
	if(carry) val |= cmask;
	}
	CFLG = carry;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e0a0(void) /* ASR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	ULONG sign = cmask & val;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		val = sign ? 0xffffffff : 0;
		CFLG=musashik_regs.x= sign ? 1 : 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
		if (sign) val |= 0xffffffff << (32 - cnt);
	}}
	NFLG = sign != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e0a8(void) /* LSR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		CFLG=musashik_regs.x = cnt == 32 ? (val & cmask ? 1 : 0) : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val >> (cnt-1)) & 1;
		val >>= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e0b0(void) /* ROXR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&1; val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e0b8(void) /* ROR */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {	ULONG carry;
	for(;cnt;--cnt){
	carry=val&1; val = val >> 1;
	if(carry) val |= cmask;
	}
	CFLG = carry;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e0d0(void) /* ASRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=val&1; val = (val >> 1) | sign;
	NFLG = sign != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e0d8(void) /* ASRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=val&1; val = (val >> 1) | sign;
	NFLG = sign != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}}
void op_e0e0(void) /* ASRW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=val&1; val = (val >> 1) | sign;
	NFLG = sign != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}}
void op_e0e8(void) /* ASRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=val&1; val = (val >> 1) | sign;
	NFLG = sign != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e0f0(void) /* ASRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=val&1; val = (val >> 1) | sign;
	NFLG = sign != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}}
void op_e0f8(void) /* ASRW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=val&1; val = (val >> 1) | sign;
	NFLG = sign != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e0f9(void) /* ASRW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=val&1; val = (val >> 1) | sign;
	NFLG = sign != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e100(void) /* ASL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		VFLG = val != 0;
		CFLG=musashik_regs.x = cnt == 8 ? val & 1 : 0;
		val = 0;
	} else {
		ULONG mask = aslmask_ubyte[cnt];
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		VFLG = (val & mask) != mask && (val & mask) != 0;
		val <<= cnt;
	}}
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e108(void) /* LSL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		CFLG=musashik_regs.x = cnt == 8 ? val & 1 : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		val <<= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e110(void) /* ROXL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e118(void) /* ROL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	ULONG carry;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(carry) val |= 1;
	}
	CFLG = carry!=0;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e120(void) /* ASL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		VFLG = val != 0;
		CFLG=musashik_regs.x = cnt == 8 ? val & 1 : 0;
		val = 0;
	} else {
		ULONG mask = aslmask_ubyte[cnt];
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		VFLG = (val & mask) != mask && (val & mask) != 0;
		val <<= cnt;
	}}
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e128(void) /* LSL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 8) {
		CFLG=musashik_regs.x = cnt == 8 ? val & 1 : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		val <<= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e130(void) /* ROXL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e138(void) /* ROL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	BYTE cnt = musashik_regs.d[srcreg].D;
{	BYTE data = musashik_regs.d[dstreg].D;
{	UBYTE val = data;
	ULONG cmask = 0x80;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	ULONG carry;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(carry) val |= 1;
	}
	CFLG = carry!=0;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].B.l = val;
}}}}}
void op_e140(void) /* ASL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		VFLG = val != 0;
		CFLG=musashik_regs.x = cnt == 16 ? val & 1 : 0;
		val = 0;
	} else {
		ULONG mask = aslmask_uword[cnt];
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		VFLG = (val & mask) != mask && (val & mask) != 0;
		val <<= cnt;
	}}
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e148(void) /* LSL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		CFLG=musashik_regs.x = cnt == 16 ? val & 1 : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		val <<= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e150(void) /* ROXL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e158(void) /* ROL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	ULONG carry;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(carry) val |= 1;
	}
	CFLG = carry!=0;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e160(void) /* ASL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		VFLG = val != 0;
		CFLG=musashik_regs.x = cnt == 16 ? val & 1 : 0;
		val = 0;
	} else {
		ULONG mask = aslmask_uword[cnt];
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		VFLG = (val & mask) != mask && (val & mask) != 0;
		val <<= cnt;
	}}
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e168(void) /* LSL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 16) {
		CFLG=musashik_regs.x = cnt == 16 ? val & 1 : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		val <<= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e170(void) /* ROXL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e178(void) /* ROL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	WORD cnt = musashik_regs.d[srcreg].D;
{	WORD data = musashik_regs.d[dstreg].D;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	ULONG carry;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(carry) val |= 1;
	}
	CFLG = carry!=0;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].W.l = val;
}}}}}
void op_e180(void) /* ASL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		VFLG = val != 0;
		CFLG=musashik_regs.x = cnt == 32 ? val & 1 : 0;
		val = 0;
	} else {
		ULONG mask = aslmask_ulong[cnt];
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		VFLG = (val & mask) != mask && (val & mask) != 0;
		val <<= cnt;
	}}
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e188(void) /* LSL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		CFLG=musashik_regs.x = cnt == 32 ? val & 1 : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		val <<= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e190(void) /* ROXL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e198(void) /* ROL */
{
	ULONG srcreg = imm8_table[((opcode >> 9) & 7)];
	ULONG dstreg = opcode & 7;
{{	ULONG cnt = srcreg;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	ULONG carry;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(carry) val |= 1;
	}
	CFLG = carry!=0;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e1a0(void) /* ASL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	VFLG = 0;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		VFLG = val != 0;
		CFLG=musashik_regs.x = cnt == 32 ? val & 1 : 0;
		val = 0;
	} else {
		ULONG mask = aslmask_ulong[cnt];
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		VFLG = (val & mask) != mask && (val & mask) != 0;
		val <<= cnt;
	}}
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e1a8(void) /* LSL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	if (cnt >= 32) {
		CFLG=musashik_regs.x = cnt == 32 ? val & 1 : 0;
		val = 0;
	} else {
		CFLG=musashik_regs.x=(val << (cnt-1)) & cmask ? 1 : 0;
		val <<= cnt;
	}}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e1b0(void) /* ROXL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	ULONG carry;
	cnt &= 63;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	}
	CFLG = musashik_regs.x;
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e1b8(void) /* ROL */
{
	ULONG srcreg = ((opcode >> 9) & 7);
	ULONG dstreg = opcode & 7;
{{	LONG cnt = musashik_regs.d[srcreg].D;
{	LONG data = musashik_regs.d[dstreg].D;
{	ULONG val = data;
	ULONG cmask = 0x80000000;
	cnt &= 63;
	if (!cnt) { CFLG = 0; } else {
	ULONG carry;
	for(;cnt;--cnt){
	carry=val&cmask; val <<= 1;
	if(carry) val |= 1;
	}
	CFLG = carry!=0;
}
	NFLG = (val & cmask) != 0; ZFLG = val == 0; VFLG = 0;
	musashik_regs.d[dstreg].D = (val);
}}}}}
void op_e1d0(void) /* ASLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=(val&cmask)!=0; val <<= 1;
	if ((val&cmask)!=sign) VFLG=1;
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e1d8(void) /* ASLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=(val&cmask)!=0; val <<= 1;
	if ((val&cmask)!=sign) VFLG=1;
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}}
void op_e1e0(void) /* ASLW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=(val&cmask)!=0; val <<= 1;
	if ((val&cmask)!=sign) VFLG=1;
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}}
void op_e1e8(void) /* ASLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=(val&cmask)!=0; val <<= 1;
	if ((val&cmask)!=sign) VFLG=1;
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e1f0(void) /* ASLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=(val&cmask)!=0; val <<= 1;
	if ((val&cmask)!=sign) VFLG=1;
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}}
void op_e1f8(void) /* ASLW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=(val&cmask)!=0; val <<= 1;
	if ((val&cmask)!=sign) VFLG=1;
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e1f9(void) /* ASLW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
	VFLG = 0;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG sign = cmask & val;
	CFLG=musashik_regs.x=(val&cmask)!=0; val <<= 1;
	if ((val&cmask)!=sign) VFLG=1;
	NFLG = (val&cmask) != 0;
	ZFLG = val == 0;
	put_word(dataa,val);
}}}}
void op_e2d0(void) /* LSRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	val >>= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e2d8(void) /* LSRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
{	UWORD val = data;
	ULONG carry = val&1;
	val >>= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e2e0(void) /* LSRW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	val >>= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e2e8(void) /* LSRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	val >>= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e2f0(void) /* LSRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	val >>= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e2f8(void) /* LSRW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	val >>= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e2f9(void) /* LSRW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	val >>= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e3d0(void) /* LSLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e3d8(void) /* LSLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e3e0(void) /* LSLW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e3e8(void) /* LSLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e3f0(void) /* LSLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e3f8(void) /* LSLW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e3f9(void) /* LSLW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = musashik_regs.x = carry!=0;
	put_word(dataa,val);
}}}}
void op_e4d0(void) /* ROXRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e4d8(void) /* ROXRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e4e0(void) /* ROXRW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e4e8(void) /* ROXRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e4f0(void) /* ROXRW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e4f8(void) /* ROXRW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e4f9(void) /* ROXRW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(musashik_regs.x) val |= cmask;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e5d0(void) /* ROXLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e5d8(void) /* ROXLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e5e0(void) /* ROXLW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e5e8(void) /* ROXLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e5f0(void) /* ROXLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e5f8(void) /* ROXLW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e5f9(void) /* ROXLW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(musashik_regs.x) val |= 1;
	musashik_regs.x = carry != 0;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
musashik_regs.x = CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e6d0(void) /* RORW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(carry) val |= cmask;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e6d8(void) /* RORW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(carry) val |= cmask;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e6e0(void) /* RORW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(carry) val |= cmask;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e6e8(void) /* RORW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(carry) val |= cmask;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e6f0(void) /* RORW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(carry) val |= cmask;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e6f8(void) /* RORW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(carry) val |= cmask;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e6f9(void) /* RORW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG carry = val&1;
	ULONG cmask = 0x8000;
	val >>= 1;
	if(carry) val |= cmask;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e7d0(void) /* ROLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(carry)  val |= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e7d8(void) /* ROLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	musashik_regs.a[srcreg] += 2;
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(carry)  val |= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e7e0(void) /* ROLW */
{
	ULONG srcreg = (opcode & 7);
{{	musashik_regs.a[srcreg] -= 2;
{	CPTR dataa = musashik_regs.a[srcreg];
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(carry)  val |= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e7e8(void) /* ROLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = musashik_regs.a[srcreg] + (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(carry)  val |= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e7f0(void) /* ROLW */
{
	ULONG srcreg = (opcode & 7);
{{	CPTR dataa = get_disp_ea(musashik_regs.a[srcreg]);
{	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(carry)  val |= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}}
void op_e7f8(void) /* ROLW */
{
{{	CPTR dataa = (LONG)(WORD)nextiword();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(carry)  val |= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
void op_e7f9(void) /* ROLW */
{
{{	CPTR dataa = nextilong();
	WORD data = get_word(dataa);
{	UWORD val = data;
	ULONG cmask = 0x8000;
	ULONG carry = val&cmask;
	val <<= 1;
	if(carry)  val |= 1;
	CLEARVC;
	ZFLG = ((WORD)(val)) == 0;
	NFLG = ((WORD)(val)) < 0;
CFLG = carry!=0;
	put_word(dataa,val);
}}}}
/* BFV 061298 - added include for instruction timing table */

/* LBO 090597 - added these lines and made them extern in cpudefs.h */
int movem_index1[256];
int movem_index2[256];
int movem_next[256];
UBYTE *actadr;

musashik_regstruct musashik_regs, lastint_musashik_regs;

union flagu intel_flag_lookup[256];
union flagu regflags;

extern int cpu_interrupt(void);
extern void BuildCPU(void);

#define MC_MUSASHI000_interrupt() (cpu_interrupt())

#define ReadMEM(A) (cpu_readmem24(A))
#define WriteMEM(A,V) (cpu_writemem24(A,V))

int MC_MUSASHI000_ICount;
int pending_interrupts;

static int InitStatus=0;

int opcode ;


void musashik_dumpstate(void)
{
}






static void initCPU(void)
{
    int i,j;

    for (i = 0 ; i < 256 ; i++) {
      for (j = 0 ; j < 8 ; j++) {
       if (i & (1 << j)) break;
      }
     movem_index1[i] = j;
     movem_index2[i] = 7-j;
     movem_next[i] = i & (~(1 << j));
    }
    for (i = 0; i < 256; i++) {
         intel_flag_lookup[i].flags.c = !!(i & 1);
         intel_flag_lookup[i].flags.z = !!(i & 64);
         intel_flag_lookup[i].flags.n = !!(i & 128);
         intel_flag_lookup[i].flags.v = 0;
    }

}

void Exception(int nr, CPTR oldpc)
{
//printf("Exception/IRQ(%i) -%i\n",nr-24,exception_cycles[nr]);
	MC_MUSASHI000_ICount -= exception_cycles[nr];

   MakeSR();

   if(!musashik_regs.s) {
   	  musashik_regs.usp=musashik_regs.a[7];
      musashik_regs.a[7]=musashik_regs.isp;
      musashik_regs.s=1;
   }

   musashik_regs.a[7] -= 4;
   put_long (musashik_regs.a[7], musashik_getpc ());
   musashik_regs.a[7] -= 2;
   put_word (musashik_regs.a[7], musashik_regs.sr);
   musashik_setpc(get_long(musashik_regs.vbr + 4*nr));

   musashik_regs.t1 = musashik_regs.t0 = musashik_regs.m = 0;
}

INLINE void Interrupt68k(int level)
{
   int ipl=musashik_regs.intmask;
   if(level>ipl)
   {
   	Exception(24+level,0);
   	pending_interrupts &= ~(1 << (level + 24));
   	musashik_regs.intmask = level;
   	MakeSR();
   }
}

void Initialisation(void) {
   /* Init 68000 emulator */
   BuildCPU();
   initCPU();
}

void MC_MUSASHI000_Reset(void)
{
if (!InitStatus)
{
	Initialisation();
	InitStatus=1;
}

   musashik_regs.a[7]=get_long(0);
   musashik_setpc(get_long(4));

   musashik_regs.s = 1;
   musashik_regs.m = 0;
   musashik_regs.stopped = 0;
   musashik_regs.t1 = 0;
   musashik_regs.t0 = 0;
   ZFLG = CFLG = NFLG = VFLG = 0;
   musashik_regs.intmask = 7;
   musashik_regs.vbr = musashik_regs.sfc = musashik_regs.dfc = 0;
   musashik_regs.fpcr = musashik_regs.fpsr = musashik_regs.fpiar = 0;

   pending_interrupts = 0;
}


void MC_MUSASHI000_SetRegs(MC_MUSASHI000_Regs *src)
{

	musashik_regs = src->musashik_regs ;

{	int sr = musashik_regs.sr ;

	NFLG = (sr >> 3) & 1;
	ZFLG = (sr >> 2) & 1;
	VFLG = (sr >> 1) & 1;
	CFLG = sr & 1;
	pending_interrupts = src->pending_interrupts;
}}

void MC_MUSASHI000_GetRegs(MC_MUSASHI000_Regs *dst)
{
   MakeSR();
	musashik_regs.sr = (musashik_regs.sr & 0xfff0) | (NFLG << 3) | (ZFLG << 2) | (VFLG << 1) | CFLG;
	dst->musashik_regs = musashik_regs;
	dst->pending_interrupts = pending_interrupts;
}


void MC_MUSASHI000_Cause_Interrupt(int level)
{
//printf("INT %i\n",level);
	if (level >= 1 && level <= 7)
		pending_interrupts |= 1 << (24 + level);
}


void MC_MUSASHI000_Clear_Pending_Interrupts(void)
{
	pending_interrupts &= ~0xff000000;
}


int  MC_MUSASHI000_GetPC(void)
{
	return musashik_regs.pc;
}



/* Execute one 68000 instruction */

int MC_MUSASHI000_Execute(int cycles)
{
	if ((pending_interrupts & MC_MUSASHI000_STOP) && !(pending_interrupts & 0xff000000))
		return cycles;

	MC_MUSASHI000_ICount = cycles;
	do
	{
		if (pending_interrupts & 0xff000000)
		{
			int level, mask = 0x80000000;
			for (level = 7; level; level--, mask >>= 1)
				if (pending_interrupts & mask)
					break;
			Interrupt68k (level);
		}

		{	/* ASG 980413 */
			extern int previouspc;
			previouspc = musashik_regs.pc;
		}

		#ifdef ASM_MEMORY
			opcode=nextiword_opcode();
		#else
			opcode=nextiword();
		#endif

		/* BFV 061298 - removed "MC_MUSASHI000_ICount -= 12;" added index lookup for timing table */
		MC_MUSASHI000_ICount -= cycletbl[opcode];
		cpufunctbl[opcode]();

	}
	while (MC_MUSASHI000_ICount > 0);

        return (cycles - MC_MUSASHI000_ICount);
}







/*
UBYTE *allocmem(long size, UBYTE init_value) {
   UBYTE *p;
   p=(UBYTE *)gp2x_malloc(size);
   return(p);
}
*/


/*
     CPU-function calls

     CPU-Emulation based on ideas by Bernd Schmidt
     Improved by Markus Gietzen
     Disassembler written by Bernd Schmidt
*/


#define CPU_EMU_SIZE 0

struct cputbl {
    cpuop_func *handler;
    int specific;
    UWORD opcode;
};


int areg_byteinc[] = {1,1,1,1,1,1,1,2};
int imm8_table[] = {8,1,2,3,4,5,6,7};
UBYTE aslmask_ubyte[] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};
UWORD aslmask_uword[] = {0x8000,0xc000,0xe000,0xf000,
                         0xf800,0xfc00,0xfe00,0xff00,
		         0xff80,0xffc0,0xffe0,0xfff0,
		         0xfff8,0xfffc,0xfffe,0xffff};
ULONG aslmask_ulong[] = {0x80000000,0xc0000000,0xe0000000,0xf0000000,
		         0xf8000000,0xfc000000,0xfe000000,0xff000000,
		         0xff800000,0xffc00000,0xffe00000,0xfff00000,
		         0xfff80000,0xfffc0000,0xfffe0000,0xffff0000,
		         0xffff8000,0xffffc000,0xffffe000,0xfffff000,
		         0xfffff800,0xfffffc00,0xfffffe00,0xffffff00,
		         0xffffff80,0xffffffc0,0xffffffe0,0xfffffff0,
		         0xfffffff8,0xfffffffc,0xfffffffe,0xffffffff};


struct instr_def defs68k[] = {
{ 60, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "ORSR.B  #1"},
{ 124, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 2, "ORSR.W  #1"},
{ 0, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "OR.z    #z,d[!Areg]"},
{ 572, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "ANDSR.B #1"},
{ 636, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 2, "ANDSR.W #1"},
{ 512, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "AND.z   #z,d[!Areg]"},
{ 1024, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "SUB.z   #z,d[!Areg]"},
{ 1536, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "ADD.z   #z,d[!Areg]"},
{ 2048, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "BTST    #1,s[!Areg]"},
{ 2112, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "BCHG    #1,s[!Areg,Immd]"},
{ 2176, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "BCLR    #1,s[!Areg,Immd]"},
{ 2240, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "BSET    #1,s[!Areg,Immd]"},
{ 2620, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "EORSR.B #1"},
{ 2684, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 2, "EORSR.W #1"},
{ 2560, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "EOR.z   #z,d[!Areg]"},
{ 3072, 8, {17,17,11,11,11,12,12,12,0,0,0,0,0,0,0,0}, 65280, 0, "CMP.z   #z,s[!Areg,Immd]"},
{ 256, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "MVPMR.W d[Areg-Ad16],Dr"},
{ 320, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "MVPMR.L d[Areg-Ad16],Dr"},
{ 384, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "MVPRM.W Dr,d[Areg-Ad16]"},
{ 448, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "MVPRM.L Dr,d[Areg-Ad16]"},
{ 256, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "BTST    Dr,s[!Areg]"},
{ 320, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "BCHG    Dr,s[!Areg,Immd]"},
{ 384, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "BCLR    Dr,s[!Areg,Immd]"},
{ 448, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "BSET    Dr,s[!Areg,Immd]"},
{ 4096, 12, {14,14,14,13,13,13,11,11,11,12,12,12,0,0,0,0}, 61440, 0, "MOVE.B  s,d[!Areg]"},
{ 8192, 12, {14,14,14,13,13,13,11,11,11,12,12,12,0,0,0,0}, 61440, 0, "MOVEA.L s,d[Areg]"},
{ 8192, 12, {14,14,14,13,13,13,11,11,11,12,12,12,0,0,0,0}, 61440, 0, "MOVE.L  s,d[!Areg]"},
{ 12288, 12, {14,14,14,13,13,13,11,11,11,12,12,12,0,0,0,0}, 61440, 0, "MOVEA.W s,d[Areg]"},
{ 12288, 12, {14,14,14,13,13,13,11,11,11,12,12,12,0,0,0,0}, 61440, 0, "MOVE.W  s,d[!Areg]"},
{ 16384, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "NEGX.z  d[!Areg]"},
{ 16576, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 1, "MVSR2.W d[!Areg]"},
{ 16896, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "CLR.z   d[!Areg]"},
{ 17408, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "NEG.z   d[!Areg]"},
{ 17600, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "MV2SR.B s[!Areg]"},
{ 17920, 8, {17,17,13,13,13,14,14,14,0,0,0,0,0,0,0,0}, 65280, 0, "NOT.z   d[!Areg]"},
{ 18112, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 2, "MV2SR.W s[!Areg]"},
{ 18432, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "NBCD.B  d[!Areg]"},
{ 18496, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "SWAP.W  s[Dreg]"},
{ 18496, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "PEA.L   s[!Dreg,Areg,Aipi,Apdi,Immd]"},
{ 18560, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "EXT.W   d[Dreg]"},
{ 18560, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "MVMLE.W #1,d[!Dreg,Areg,Aipi]"},
{ 18624, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "EXT.L   d[Dreg]"},
{ 18624, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "MVMLE.L #1,d[!Dreg,Areg,Aipi]"},
{ 18880, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "EXT.B   d[Dreg]"},
{ 18944, 8, {17,17,11,11,11,12,12,12,0,0,0,0,0,0,0,0}, 65280, 0, "TST.z   s"},
{ 19136, 6, {13,13,13,14,14,14,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "TAS.B   d[!Areg]"},
{ 19196, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "ILLEGAL"},
{ 19584, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "MVMEL.W #1,s[!Dreg,Areg,Apdi,Immd]"},
{ 19648, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "MVMEL.L #1,s[!Dreg,Areg,Apdi,Immd]"},
{ 20032, 4, {8,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0}, 65520, 0, "TRAP    #J"},
{ 20048, 3, {15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65528, 0, "LINK.W  Ar,#1"},
{ 20056, 3, {15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65528, 0, "UNLK.L  Ar"},
{ 20064, 3, {15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65528, 2, "MVR2USP.L Ar"},
{ 20072, 3, {15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65528, 2, "MVUSP2R.L Ar"},
{ 20080, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 2, "RESET"},
{ 20081, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "NOP"},
{ 20082, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 2, "STOP    #1"},
{ 20083, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 2, "RTE"},
{ 20084, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "RTD     #1"},
{ 20085, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "RTS"},
{ 20086, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "TRAPV"},
{ 20087, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "RTR"},
{ 20096, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "JSR.L   s[!Dreg,Areg,Aipi,Apdi,Immd]"},
{ 16640, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "CHK.L   s[!Areg],Dr"},
{ 16768, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "CHK.W   s[!Areg],Dr"},
{ 20160, 6, {11,11,11,12,12,12,0,0,0,0,0,0,0,0,0,0}, 65472, 0, "JMP.L   s[!Dreg,Areg,Aipi,Apdi,Immd]"},
{ 16832, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "LEA.L   s[!Dreg,Areg,Aipi,Apdi,Immd],Ar"},
{ 20480, 11, {7,7,7,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "ADDA.z  #j,d[Areg]"},
{ 20480, 11, {7,7,7,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "ADD.z   #j,d[!Areg]"},
{ 20736, 11, {7,7,7,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "SUBA.z  #j,d[Areg]"},
{ 20736, 11, {7,7,7,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "SUB.z   #j,d[!Areg]"},
{ 20680, 7, {2,2,2,2,15,15,15,0,0,0,0,0,0,0,0,0}, 61688, 0, "DBcc.W  Dr,#1"},
{ 20672, 10, {2,2,2,2,13,13,13,14,14,14,0,0,0,0,0,0}, 61632, 0, "Scc.B   d[!Areg]"},
{ 24832, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "BSR.W   #1"},
{ 24832, 8, {6,6,6,6,6,6,6,6,0,0,0,0,0,0,0,0}, 65280, 0, "BSR.B   #i"},
{ 25087, 0, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 65535, 0, "BSR.L   #2"},
{ 24576, 4, {3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0}, 61695, 0, "Bcc.W   #1"},
{ 24576, 12, {3,3,3,3,6,6,6,6,6,6,6,6,0,0,0,0}, 61440, 0, "Bcc.B   #i"},
{ 24831, 4, {3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0}, 61695, 0, "Bcc.L   #2"},
{ 28672, 11, {15,15,15,5,5,5,5,5,5,5,5,0,0,0,0,0}, 61696, 0, "MOVE.L  #i,Dr"},
{ 32768, 11, {15,15,15,17,17,11,11,11,12,12,12,0,0,0,0,0}, 61696, 0, "OR.z    s[!Areg],Dr"},
{ 32960, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "DIVU.W  s[!Areg],Dr"},
{ 33024, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "SBCD.B  d[Dreg],Dr"},
{ 33024, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "SBCD.B  d[Areg-Apdi],Arp"},
{ 33024, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "OR.z    Dr,d[!Areg,Dreg]"},
{ 33216, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "DIVS.W  s[!Areg],Dr"},
{ 36864, 11, {15,15,15,17,17,11,11,11,12,12,12,0,0,0,0,0}, 61696, 0, "SUB.z   s,Dr"},
{ 37056, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "SUBA.W  s,Ar"},
{ 37120, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "SUBX.z  d[Dreg],Dr"},
{ 37120, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "SUBX.z  d[Areg-Apdi],Arp"},
{ 37120, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "SUB.z   Dr,d[!Areg,Dreg]"},
{ 37312, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "SUBA.L  s,Ar"},
{ 45056, 11, {15,15,15,17,17,11,11,11,12,12,12,0,0,0,0,0}, 61696, 0, "CMP.z   s,Dr"},
{ 45248, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "CMPA.W  s,Ar"},
{ 45504, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "CMPA.L  s,Ar"},
{ 45312, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "CMPM.z  d[Areg-Aipi],ArP"},
{ 45312, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "EOR.z   Dr,d[!Areg]"},
{ 49152, 11, {15,15,15,17,17,11,11,11,12,12,12,0,0,0,0,0}, 61696, 0, "AND.z   s[!Areg],Dr"},
{ 49344, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "MULU.W  s[!Areg],Dr"},
{ 49408, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "ABCD.B  d[Dreg],Dr"},
{ 49408, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "ABCD.B  d[Areg-Apdi],Arp"},
{ 49408, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "AND.z   Dr,d[!Areg,Dreg]"},
{ 49472, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "EXG.L   Dr,d[Dreg]"},
{ 49472, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "EXG.L   Ar,d[Areg]"},
{ 49536, 9, {15,15,15,13,13,13,14,14,14,0,0,0,0,0,0,0}, 61888, 0, "EXG.L   Dr,d[Areg]"},
{ 49600, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "MULS.W  s[!Areg],Dr"},
{ 53248, 11, {15,15,15,17,17,11,11,11,12,12,12,0,0,0,0,0}, 61696, 0, "ADD.z   s,Dr"},
{ 53440, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "ADDA.W  s,Ar"},
{ 53504, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "ADDX.z  d[Dreg],Dr"},
{ 53504, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "ADDX.z  d[Areg-Apdi],Arp"},
{ 53504, 11, {15,15,15,17,17,13,13,13,14,14,14,0,0,0,0,0}, 61696, 0, "ADD.z   Dr,d[!Areg,Dreg]"},
{ 53696, 9, {15,15,15,11,11,11,12,12,12,0,0,0,0,0,0,0}, 61888, 0, "ADDA.L  s,Ar"},
{ 57344, 9, {7,7,7,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "ASf.z   #j,DR"},
{ 57352, 9, {7,7,7,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "LSf.z   #j,DR"},
{ 57360, 9, {7,7,7,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "ROXf.z  #j,DR"},
{ 57368, 9, {7,7,7,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "ROf.z   #j,DR"},
{ 57376, 9, {15,15,15,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "ASf.z   Dr,DR"},
{ 57384, 9, {15,15,15,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "LSf.z   Dr,DR"},
{ 57392, 9, {15,15,15,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "ROXf.z  Dr,DR"},
{ 57400, 9, {15,15,15,4,17,17,16,16,16,0,0,0,0,0,0,0}, 61496, 0, "ROf.z   Dr,DR"},
{ 57536, 7, {4,13,13,13,14,14,14,0,0,0,0,0,0,0,0,0}, 65216, 0, "ASfW.W  d[!Dreg,Areg]"},
{ 58048, 7, {4,13,13,13,14,14,14,0,0,0,0,0,0,0,0,0}, 65216, 0, "LSfW.W  d[!Dreg,Areg]"},
{ 58560, 7, {4,13,13,13,14,14,14,0,0,0,0,0,0,0,0,0}, 65216, 0, "ROXfW.W d[!Dreg,Areg]"},
{ 59072, 7, {4,13,13,13,14,14,14,0,0,0,0,0,0,0,0,0}, 65216, 0, "ROfW.W  d[!Dreg,Areg]"}};
int n_defs68k=124;

struct cputbl smallcputbl[] = {
{ op_0, 0, 0 }, /* OR */
{ op_10, 0, 16 }, /* OR */
{ op_18, 0, 24 }, /* OR */
{ op_20, 0, 32 }, /* OR */
{ op_28, 0, 40 }, /* OR */
{ op_30, 0, 48 }, /* OR */
{ op_38, 0, 56 }, /* OR */
{ op_39, 0, 57 }, /* OR */
{ op_3c, 0, 60 }, /* ORSR */
{ op_40, 0, 64 }, /* OR */
{ op_50, 0, 80 }, /* OR */
{ op_58, 0, 88 }, /* OR */
{ op_60, 0, 96 }, /* OR */
{ op_68, 0, 104 }, /* OR */
{ op_70, 0, 112 }, /* OR */
{ op_78, 0, 120 }, /* OR */
{ op_79, 0, 121 }, /* OR */
{ op_7c, 0, 124 }, /* ORSR */
{ op_80, 0, 128 }, /* OR */
{ op_90, 0, 144 }, /* OR */
{ op_98, 0, 152 }, /* OR */
{ op_a0, 0, 160 }, /* OR */
{ op_a8, 0, 168 }, /* OR */
{ op_b0, 0, 176 }, /* OR */
{ op_b8, 0, 184 }, /* OR */
{ op_b9, 0, 185 }, /* OR */
{ op_100, 0, 256 }, /* BTST */
{ op_108, 0, 264 }, /* MVPMR */
{ op_110, 0, 272 }, /* BTST */
{ op_118, 0, 280 }, /* BTST */
{ op_120, 0, 288 }, /* BTST */
{ op_128, 0, 296 }, /* BTST */
{ op_130, 0, 304 }, /* BTST */
{ op_138, 0, 312 }, /* BTST */
{ op_139, 0, 313 }, /* BTST */
{ op_13a, 0, 314 }, /* BTST */
{ op_13b, 0, 315 }, /* BTST */
{ op_13c, 0, 316 }, /* BTST */
{ op_140, 0, 320 }, /* BCHG */
{ op_148, 0, 328 }, /* MVPMR */
{ op_150, 0, 336 }, /* BCHG */
{ op_158, 0, 344 }, /* BCHG */
{ op_160, 0, 352 }, /* BCHG */
{ op_168, 0, 360 }, /* BCHG */
{ op_170, 0, 368 }, /* BCHG */
{ op_178, 0, 376 }, /* BCHG */
{ op_179, 0, 377 }, /* BCHG */
{ op_17a, 0, 378 }, /* BCHG */
{ op_17b, 0, 379 }, /* BCHG */
{ op_180, 0, 384 }, /* BCLR */
{ op_188, 0, 392 }, /* MVPRM */
{ op_190, 0, 400 }, /* BCLR */
{ op_198, 0, 408 }, /* BCLR */
{ op_1a0, 0, 416 }, /* BCLR */
{ op_1a8, 0, 424 }, /* BCLR */
{ op_1b0, 0, 432 }, /* BCLR */
{ op_1b8, 0, 440 }, /* BCLR */
{ op_1b9, 0, 441 }, /* BCLR */
{ op_1ba, 0, 442 }, /* BCLR */
{ op_1bb, 0, 443 }, /* BCLR */
{ op_1c0, 0, 448 }, /* BSET */
{ op_1c8, 0, 456 }, /* MVPRM */
{ op_1d0, 0, 464 }, /* BSET */
{ op_1d8, 0, 472 }, /* BSET */
{ op_1e0, 0, 480 }, /* BSET */
{ op_1e8, 0, 488 }, /* BSET */
{ op_1f0, 0, 496 }, /* BSET */
{ op_1f8, 0, 504 }, /* BSET */
{ op_1f9, 0, 505 }, /* BSET */
{ op_1fa, 0, 506 }, /* BSET */
{ op_1fb, 0, 507 }, /* BSET */
{ op_200, 0, 512 }, /* AND */
{ op_210, 0, 528 }, /* AND */
{ op_218, 0, 536 }, /* AND */
{ op_220, 0, 544 }, /* AND */
{ op_228, 0, 552 }, /* AND */
{ op_230, 0, 560 }, /* AND */
{ op_238, 0, 568 }, /* AND */
{ op_239, 0, 569 }, /* AND */
{ op_23c, 0, 572 }, /* ANDSR */
{ op_240, 0, 576 }, /* AND */
{ op_250, 0, 592 }, /* AND */
{ op_258, 0, 600 }, /* AND */
{ op_260, 0, 608 }, /* AND */
{ op_268, 0, 616 }, /* AND */
{ op_270, 0, 624 }, /* AND */
{ op_278, 0, 632 }, /* AND */
{ op_279, 0, 633 }, /* AND */
{ op_27c, 0, 636 }, /* ANDSR */
{ op_280, 0, 640 }, /* AND */
{ op_290, 0, 656 }, /* AND */
{ op_298, 0, 664 }, /* AND */
{ op_2a0, 0, 672 }, /* AND */
{ op_2a8, 0, 680 }, /* AND */
{ op_2b0, 0, 688 }, /* AND */
{ op_2b8, 0, 696 }, /* AND */
{ op_2b9, 0, 697 }, /* AND */
{ op_400, 0, 1024 }, /* SUB */
{ op_410, 0, 1040 }, /* SUB */
{ op_418, 0, 1048 }, /* SUB */
{ op_420, 0, 1056 }, /* SUB */
{ op_428, 0, 1064 }, /* SUB */
{ op_430, 0, 1072 }, /* SUB */
{ op_438, 0, 1080 }, /* SUB */
{ op_439, 0, 1081 }, /* SUB */
{ op_440, 0, 1088 }, /* SUB */
{ op_450, 0, 1104 }, /* SUB */
{ op_458, 0, 1112 }, /* SUB */
{ op_460, 0, 1120 }, /* SUB */
{ op_468, 0, 1128 }, /* SUB */
{ op_470, 0, 1136 }, /* SUB */
{ op_478, 0, 1144 }, /* SUB */
{ op_479, 0, 1145 }, /* SUB */
{ op_480, 0, 1152 }, /* SUB */
{ op_490, 0, 1168 }, /* SUB */
{ op_498, 0, 1176 }, /* SUB */
{ op_4a0, 0, 1184 }, /* SUB */
{ op_4a8, 0, 1192 }, /* SUB */
{ op_4b0, 0, 1200 }, /* SUB */
{ op_4b8, 0, 1208 }, /* SUB */
{ op_4b9, 0, 1209 }, /* SUB */
{ op_600, 0, 1536 }, /* ADD */
{ op_610, 0, 1552 }, /* ADD */
{ op_618, 0, 1560 }, /* ADD */
{ op_620, 0, 1568 }, /* ADD */
{ op_628, 0, 1576 }, /* ADD */
{ op_630, 0, 1584 }, /* ADD */
{ op_638, 0, 1592 }, /* ADD */
{ op_639, 0, 1593 }, /* ADD */
{ op_640, 0, 1600 }, /* ADD */
{ op_650, 0, 1616 }, /* ADD */
{ op_658, 0, 1624 }, /* ADD */
{ op_660, 0, 1632 }, /* ADD */
{ op_668, 0, 1640 }, /* ADD */
{ op_670, 0, 1648 }, /* ADD */
{ op_678, 0, 1656 }, /* ADD */
{ op_679, 0, 1657 }, /* ADD */
{ op_680, 0, 1664 }, /* ADD */
{ op_690, 0, 1680 }, /* ADD */
{ op_698, 0, 1688 }, /* ADD */
{ op_6a0, 0, 1696 }, /* ADD */
{ op_6a8, 0, 1704 }, /* ADD */
{ op_6b0, 0, 1712 }, /* ADD */
{ op_6b8, 0, 1720 }, /* ADD */
{ op_6b9, 0, 1721 }, /* ADD */
{ op_800, 0, 2048 }, /* BTST */
{ op_810, 0, 2064 }, /* BTST */
{ op_818, 0, 2072 }, /* BTST */
{ op_820, 0, 2080 }, /* BTST */
{ op_828, 0, 2088 }, /* BTST */
{ op_830, 0, 2096 }, /* BTST */
{ op_838, 0, 2104 }, /* BTST */
{ op_839, 0, 2105 }, /* BTST */
{ op_83a, 0, 2106 }, /* BTST */
{ op_83b, 0, 2107 }, /* BTST */
{ op_83c, 0, 2108 }, /* BTST */
{ op_840, 0, 2112 }, /* BCHG */
{ op_850, 0, 2128 }, /* BCHG */
{ op_858, 0, 2136 }, /* BCHG */
{ op_860, 0, 2144 }, /* BCHG */
{ op_868, 0, 2152 }, /* BCHG */
{ op_870, 0, 2160 }, /* BCHG */
{ op_878, 0, 2168 }, /* BCHG */
{ op_879, 0, 2169 }, /* BCHG */
{ op_87a, 0, 2170 }, /* BCHG */
{ op_87b, 0, 2171 }, /* BCHG */
{ op_880, 0, 2176 }, /* BCLR */
{ op_890, 0, 2192 }, /* BCLR */
{ op_898, 0, 2200 }, /* BCLR */
{ op_8a0, 0, 2208 }, /* BCLR */
{ op_8a8, 0, 2216 }, /* BCLR */
{ op_8b0, 0, 2224 }, /* BCLR */
{ op_8b8, 0, 2232 }, /* BCLR */
{ op_8b9, 0, 2233 }, /* BCLR */
{ op_8ba, 0, 2234 }, /* BCLR */
{ op_8bb, 0, 2235 }, /* BCLR */
{ op_8c0, 0, 2240 }, /* BSET */
{ op_8d0, 0, 2256 }, /* BSET */
{ op_8d8, 0, 2264 }, /* BSET */
{ op_8e0, 0, 2272 }, /* BSET */
{ op_8e8, 0, 2280 }, /* BSET */
{ op_8f0, 0, 2288 }, /* BSET */
{ op_8f8, 0, 2296 }, /* BSET */
{ op_8f9, 0, 2297 }, /* BSET */
{ op_8fa, 0, 2298 }, /* BSET */
{ op_8fb, 0, 2299 }, /* BSET */
{ op_a00, 0, 2560 }, /* EOR */
{ op_a10, 0, 2576 }, /* EOR */
{ op_a18, 0, 2584 }, /* EOR */
{ op_a20, 0, 2592 }, /* EOR */
{ op_a28, 0, 2600 }, /* EOR */
{ op_a30, 0, 2608 }, /* EOR */
{ op_a38, 0, 2616 }, /* EOR */
{ op_a39, 0, 2617 }, /* EOR */
{ op_a3c, 0, 2620 }, /* EORSR */
{ op_a40, 0, 2624 }, /* EOR */
{ op_a50, 0, 2640 }, /* EOR */
{ op_a58, 0, 2648 }, /* EOR */
{ op_a60, 0, 2656 }, /* EOR */
{ op_a68, 0, 2664 }, /* EOR */
{ op_a70, 0, 2672 }, /* EOR */
{ op_a78, 0, 2680 }, /* EOR */
{ op_a79, 0, 2681 }, /* EOR */
{ op_a7c, 0, 2684 }, /* EORSR */
{ op_a80, 0, 2688 }, /* EOR */
{ op_a90, 0, 2704 }, /* EOR */
{ op_a98, 0, 2712 }, /* EOR */
{ op_aa0, 0, 2720 }, /* EOR */
{ op_aa8, 0, 2728 }, /* EOR */
{ op_ab0, 0, 2736 }, /* EOR */
{ op_ab8, 0, 2744 }, /* EOR */
{ op_ab9, 0, 2745 }, /* EOR */
{ op_c00, 0, 3072 }, /* CMP */
{ op_c10, 0, 3088 }, /* CMP */
{ op_c18, 0, 3096 }, /* CMP */
{ op_c20, 0, 3104 }, /* CMP */
{ op_c28, 0, 3112 }, /* CMP */
{ op_c30, 0, 3120 }, /* CMP */
{ op_c38, 0, 3128 }, /* CMP */
{ op_c39, 0, 3129 }, /* CMP */
{ op_c3a, 0, 3130 }, /* CMP */
{ op_c3b, 0, 3131 }, /* CMP */
{ op_c40, 0, 3136 }, /* CMP */
{ op_c50, 0, 3152 }, /* CMP */
{ op_c58, 0, 3160 }, /* CMP */
{ op_c60, 0, 3168 }, /* CMP */
{ op_c68, 0, 3176 }, /* CMP */
{ op_c70, 0, 3184 }, /* CMP */
{ op_c78, 0, 3192 }, /* CMP */
{ op_c79, 0, 3193 }, /* CMP */
{ op_c7a, 0, 3194 }, /* CMP */
{ op_c7b, 0, 3195 }, /* CMP */
{ op_c80, 0, 3200 }, /* CMP */
{ op_c90, 0, 3216 }, /* CMP */
{ op_c98, 0, 3224 }, /* CMP */
{ op_ca0, 0, 3232 }, /* CMP */
{ op_ca8, 0, 3240 }, /* CMP */
{ op_cb0, 0, 3248 }, /* CMP */
{ op_cb8, 0, 3256 }, /* CMP */
{ op_cb9, 0, 3257 }, /* CMP */
{ op_cba, 0, 3258 }, /* CMP */
{ op_cbb, 0, 3259 }, /* CMP */
{ op_1000, 0, 4096 }, /* MOVE */
{ op_1010, 0, 4112 }, /* MOVE */
{ op_1018, 0, 4120 }, /* MOVE */
{ op_1020, 0, 4128 }, /* MOVE */
{ op_1028, 0, 4136 }, /* MOVE */
{ op_1030, 0, 4144 }, /* MOVE */
{ op_1038, 0, 4152 }, /* MOVE */
{ op_1039, 0, 4153 }, /* MOVE */
{ op_103a, 0, 4154 }, /* MOVE */
{ op_103b, 0, 4155 }, /* MOVE */
{ op_103c, 0, 4156 }, /* MOVE */
{ op_1080, 0, 4224 }, /* MOVE */
{ op_1090, 0, 4240 }, /* MOVE */
{ op_1098, 0, 4248 }, /* MOVE */
{ op_10a0, 0, 4256 }, /* MOVE */
{ op_10a8, 0, 4264 }, /* MOVE */
{ op_10b0, 0, 4272 }, /* MOVE */
{ op_10b8, 0, 4280 }, /* MOVE */
{ op_10b9, 0, 4281 }, /* MOVE */
{ op_10ba, 0, 4282 }, /* MOVE */
{ op_10bb, 0, 4283 }, /* MOVE */
{ op_10bc, 0, 4284 }, /* MOVE */
{ op_10c0, 0, 4288 }, /* MOVE */
{ op_10d0, 0, 4304 }, /* MOVE */
{ op_10d8, 0, 4312 }, /* MOVE */
{ op_10e0, 0, 4320 }, /* MOVE */
{ op_10e8, 0, 4328 }, /* MOVE */
{ op_10f0, 0, 4336 }, /* MOVE */
{ op_10f8, 0, 4344 }, /* MOVE */
{ op_10f9, 0, 4345 }, /* MOVE */
{ op_10fa, 0, 4346 }, /* MOVE */
{ op_10fb, 0, 4347 }, /* MOVE */
{ op_10fc, 0, 4348 }, /* MOVE */
{ op_1100, 0, 4352 }, /* MOVE */
{ op_1110, 0, 4368 }, /* MOVE */
{ op_1118, 0, 4376 }, /* MOVE */
{ op_1120, 0, 4384 }, /* MOVE */
{ op_1128, 0, 4392 }, /* MOVE */
{ op_1130, 0, 4400 }, /* MOVE */
{ op_1138, 0, 4408 }, /* MOVE */
{ op_1139, 0, 4409 }, /* MOVE */
{ op_113a, 0, 4410 }, /* MOVE */
{ op_113b, 0, 4411 }, /* MOVE */
{ op_113c, 0, 4412 }, /* MOVE */
{ op_1140, 0, 4416 }, /* MOVE */
{ op_1150, 0, 4432 }, /* MOVE */
{ op_1158, 0, 4440 }, /* MOVE */
{ op_1160, 0, 4448 }, /* MOVE */
{ op_1168, 0, 4456 }, /* MOVE */
{ op_1170, 0, 4464 }, /* MOVE */
{ op_1178, 0, 4472 }, /* MOVE */
{ op_1179, 0, 4473 }, /* MOVE */
{ op_117a, 0, 4474 }, /* MOVE */
{ op_117b, 0, 4475 }, /* MOVE */
{ op_117c, 0, 4476 }, /* MOVE */
{ op_1180, 0, 4480 }, /* MOVE */
{ op_1190, 0, 4496 }, /* MOVE */
{ op_1198, 0, 4504 }, /* MOVE */
{ op_11a0, 0, 4512 }, /* MOVE */
{ op_11a8, 0, 4520 }, /* MOVE */
{ op_11b0, 0, 4528 }, /* MOVE */
{ op_11b8, 0, 4536 }, /* MOVE */
{ op_11b9, 0, 4537 }, /* MOVE */
{ op_11ba, 0, 4538 }, /* MOVE */
{ op_11bb, 0, 4539 }, /* MOVE */
{ op_11bc, 0, 4540 }, /* MOVE */
{ op_11c0, 0, 4544 }, /* MOVE */
{ op_11d0, 0, 4560 }, /* MOVE */
{ op_11d8, 0, 4568 }, /* MOVE */
{ op_11e0, 0, 4576 }, /* MOVE */
{ op_11e8, 0, 4584 }, /* MOVE */
{ op_11f0, 0, 4592 }, /* MOVE */
{ op_11f8, 0, 4600 }, /* MOVE */
{ op_11f9, 0, 4601 }, /* MOVE */
{ op_11fa, 0, 4602 }, /* MOVE */
{ op_11fb, 0, 4603 }, /* MOVE */
{ op_11fc, 0, 4604 }, /* MOVE */
{ op_13c0, 0, 5056 }, /* MOVE */
{ op_13d0, 0, 5072 }, /* MOVE */
{ op_13d8, 0, 5080 }, /* MOVE */
{ op_13e0, 0, 5088 }, /* MOVE */
{ op_13e8, 0, 5096 }, /* MOVE */
{ op_13f0, 0, 5104 }, /* MOVE */
{ op_13f8, 0, 5112 }, /* MOVE */
{ op_13f9, 0, 5113 }, /* MOVE */
{ op_13fa, 0, 5114 }, /* MOVE */
{ op_13fb, 0, 5115 }, /* MOVE */
{ op_13fc, 0, 5116 }, /* MOVE */
{ op_2000, 0, 8192 }, /* MOVE */
{ op_2008, 0, 8200 }, /* MOVE */
{ op_2010, 0, 8208 }, /* MOVE */
{ op_2018, 0, 8216 }, /* MOVE */
{ op_2020, 0, 8224 }, /* MOVE */
{ op_2028, 0, 8232 }, /* MOVE */
{ op_2030, 0, 8240 }, /* MOVE */
{ op_2038, 0, 8248 }, /* MOVE */
{ op_2039, 0, 8249 }, /* MOVE */
{ op_203a, 0, 8250 }, /* MOVE */
{ op_203b, 0, 8251 }, /* MOVE */
{ op_203c, 0, 8252 }, /* MOVE */
{ op_2040, 0, 8256 }, /* MOVEA */
{ op_2048, 0, 8264 }, /* MOVEA */
{ op_2050, 0, 8272 }, /* MOVEA */
{ op_2058, 0, 8280 }, /* MOVEA */
{ op_2060, 0, 8288 }, /* MOVEA */
{ op_2068, 0, 8296 }, /* MOVEA */
{ op_2070, 0, 8304 }, /* MOVEA */
{ op_2078, 0, 8312 }, /* MOVEA */
{ op_2079, 0, 8313 }, /* MOVEA */
{ op_207a, 0, 8314 }, /* MOVEA */
{ op_207b, 0, 8315 }, /* MOVEA */
{ op_207c, 0, 8316 }, /* MOVEA */
{ op_2080, 0, 8320 }, /* MOVE */
{ op_2088, 0, 8328 }, /* MOVE */
{ op_2090, 0, 8336 }, /* MOVE */
{ op_2098, 0, 8344 }, /* MOVE */
{ op_20a0, 0, 8352 }, /* MOVE */
{ op_20a8, 0, 8360 }, /* MOVE */
{ op_20b0, 0, 8368 }, /* MOVE */
{ op_20b8, 0, 8376 }, /* MOVE */
{ op_20b9, 0, 8377 }, /* MOVE */
{ op_20ba, 0, 8378 }, /* MOVE */
{ op_20bb, 0, 8379 }, /* MOVE */
{ op_20bc, 0, 8380 }, /* MOVE */
{ op_20c0, 0, 8384 }, /* MOVE */
{ op_20c8, 0, 8392 }, /* MOVE */
{ op_20d0, 0, 8400 }, /* MOVE */
{ op_20d8, 0, 8408 }, /* MOVE */
{ op_20e0, 0, 8416 }, /* MOVE */
{ op_20e8, 0, 8424 }, /* MOVE */
{ op_20f0, 0, 8432 }, /* MOVE */
{ op_20f8, 0, 8440 }, /* MOVE */
{ op_20f9, 0, 8441 }, /* MOVE */
{ op_20fa, 0, 8442 }, /* MOVE */
{ op_20fb, 0, 8443 }, /* MOVE */
{ op_20fc, 0, 8444 }, /* MOVE */
{ op_2100, 0, 8448 }, /* MOVE */
{ op_2108, 0, 8456 }, /* MOVE */
{ op_2110, 0, 8464 }, /* MOVE */
{ op_2118, 0, 8472 }, /* MOVE */
{ op_2120, 0, 8480 }, /* MOVE */
{ op_2128, 0, 8488 }, /* MOVE */
{ op_2130, 0, 8496 }, /* MOVE */
{ op_2138, 0, 8504 }, /* MOVE */
{ op_2139, 0, 8505 }, /* MOVE */
{ op_213a, 0, 8506 }, /* MOVE */
{ op_213b, 0, 8507 }, /* MOVE */
{ op_213c, 0, 8508 }, /* MOVE */
{ op_2140, 0, 8512 }, /* MOVE */
{ op_2148, 0, 8520 }, /* MOVE */
{ op_2150, 0, 8528 }, /* MOVE */
{ op_2158, 0, 8536 }, /* MOVE */
{ op_2160, 0, 8544 }, /* MOVE */
{ op_2168, 0, 8552 }, /* MOVE */
{ op_2170, 0, 8560 }, /* MOVE */
{ op_2178, 0, 8568 }, /* MOVE */
{ op_2179, 0, 8569 }, /* MOVE */
{ op_217a, 0, 8570 }, /* MOVE */
{ op_217b, 0, 8571 }, /* MOVE */
{ op_217c, 0, 8572 }, /* MOVE */
{ op_2180, 0, 8576 }, /* MOVE */
{ op_2188, 0, 8584 }, /* MOVE */
{ op_2190, 0, 8592 }, /* MOVE */
{ op_2198, 0, 8600 }, /* MOVE */
{ op_21a0, 0, 8608 }, /* MOVE */
{ op_21a8, 0, 8616 }, /* MOVE */
{ op_21b0, 0, 8624 }, /* MOVE */
{ op_21b8, 0, 8632 }, /* MOVE */
{ op_21b9, 0, 8633 }, /* MOVE */
{ op_21ba, 0, 8634 }, /* MOVE */
{ op_21bb, 0, 8635 }, /* MOVE */
{ op_21bc, 0, 8636 }, /* MOVE */
{ op_21c0, 0, 8640 }, /* MOVE */
{ op_21c8, 0, 8648 }, /* MOVE */
{ op_21d0, 0, 8656 }, /* MOVE */
{ op_21d8, 0, 8664 }, /* MOVE */
{ op_21e0, 0, 8672 }, /* MOVE */
{ op_21e8, 0, 8680 }, /* MOVE */
{ op_21f0, 0, 8688 }, /* MOVE */
{ op_21f8, 0, 8696 }, /* MOVE */
{ op_21f9, 0, 8697 }, /* MOVE */
{ op_21fa, 0, 8698 }, /* MOVE */
{ op_21fb, 0, 8699 }, /* MOVE */
{ op_21fc, 0, 8700 }, /* MOVE */
{ op_23c0, 0, 9152 }, /* MOVE */
{ op_23c8, 0, 9160 }, /* MOVE */
{ op_23d0, 0, 9168 }, /* MOVE */
{ op_23d8, 0, 9176 }, /* MOVE */
{ op_23e0, 0, 9184 }, /* MOVE */
{ op_23e8, 0, 9192 }, /* MOVE */
{ op_23f0, 0, 9200 }, /* MOVE */
{ op_23f8, 0, 9208 }, /* MOVE */
{ op_23f9, 0, 9209 }, /* MOVE */
{ op_23fa, 0, 9210 }, /* MOVE */
{ op_23fb, 0, 9211 }, /* MOVE */
{ op_23fc, 0, 9212 }, /* MOVE */
{ op_3000, 0, 12288 }, /* MOVE */
{ op_3008, 0, 12296 }, /* MOVE */
{ op_3010, 0, 12304 }, /* MOVE */
{ op_3018, 0, 12312 }, /* MOVE */
{ op_3020, 0, 12320 }, /* MOVE */
{ op_3028, 0, 12328 }, /* MOVE */
{ op_3030, 0, 12336 }, /* MOVE */
{ op_3038, 0, 12344 }, /* MOVE */
{ op_3039, 0, 12345 }, /* MOVE */
{ op_303a, 0, 12346 }, /* MOVE */
{ op_303b, 0, 12347 }, /* MOVE */
{ op_303c, 0, 12348 }, /* MOVE */
{ op_3040, 0, 12352 }, /* MOVEA */
{ op_3048, 0, 12360 }, /* MOVEA */
{ op_3050, 0, 12368 }, /* MOVEA */
{ op_3058, 0, 12376 }, /* MOVEA */
{ op_3060, 0, 12384 }, /* MOVEA */
{ op_3068, 0, 12392 }, /* MOVEA */
{ op_3070, 0, 12400 }, /* MOVEA */
{ op_3078, 0, 12408 }, /* MOVEA */
{ op_3079, 0, 12409 }, /* MOVEA */
{ op_307a, 0, 12410 }, /* MOVEA */
{ op_307b, 0, 12411 }, /* MOVEA */
{ op_307c, 0, 12412 }, /* MOVEA */
{ op_3080, 0, 12416 }, /* MOVE */
{ op_3088, 0, 12424 }, /* MOVE */
{ op_3090, 0, 12432 }, /* MOVE */
{ op_3098, 0, 12440 }, /* MOVE */
{ op_30a0, 0, 12448 }, /* MOVE */
{ op_30a8, 0, 12456 }, /* MOVE */
{ op_30b0, 0, 12464 }, /* MOVE */
{ op_30b8, 0, 12472 }, /* MOVE */
{ op_30b9, 0, 12473 }, /* MOVE */
{ op_30ba, 0, 12474 }, /* MOVE */
{ op_30bb, 0, 12475 }, /* MOVE */
{ op_30bc, 0, 12476 }, /* MOVE */
{ op_30c0, 0, 12480 }, /* MOVE */
{ op_30c8, 0, 12488 }, /* MOVE */
{ op_30d0, 0, 12496 }, /* MOVE */
{ op_30d8, 0, 12504 }, /* MOVE */
{ op_30e0, 0, 12512 }, /* MOVE */
{ op_30e8, 0, 12520 }, /* MOVE */
{ op_30f0, 0, 12528 }, /* MOVE */
{ op_30f8, 0, 12536 }, /* MOVE */
{ op_30f9, 0, 12537 }, /* MOVE */
{ op_30fa, 0, 12538 }, /* MOVE */
{ op_30fb, 0, 12539 }, /* MOVE */
{ op_30fc, 0, 12540 }, /* MOVE */
{ op_3100, 0, 12544 }, /* MOVE */
{ op_3108, 0, 12552 }, /* MOVE */
{ op_3110, 0, 12560 }, /* MOVE */
{ op_3118, 0, 12568 }, /* MOVE */
{ op_3120, 0, 12576 }, /* MOVE */
{ op_3128, 0, 12584 }, /* MOVE */
{ op_3130, 0, 12592 }, /* MOVE */
{ op_3138, 0, 12600 }, /* MOVE */
{ op_3139, 0, 12601 }, /* MOVE */
{ op_313a, 0, 12602 }, /* MOVE */
{ op_313b, 0, 12603 }, /* MOVE */
{ op_313c, 0, 12604 }, /* MOVE */
{ op_3140, 0, 12608 }, /* MOVE */
{ op_3148, 0, 12616 }, /* MOVE */
{ op_3150, 0, 12624 }, /* MOVE */
{ op_3158, 0, 12632 }, /* MOVE */
{ op_3160, 0, 12640 }, /* MOVE */
{ op_3168, 0, 12648 }, /* MOVE */
{ op_3170, 0, 12656 }, /* MOVE */
{ op_3178, 0, 12664 }, /* MOVE */
{ op_3179, 0, 12665 }, /* MOVE */
{ op_317a, 0, 12666 }, /* MOVE */
{ op_317b, 0, 12667 }, /* MOVE */
{ op_317c, 0, 12668 }, /* MOVE */
{ op_3180, 0, 12672 }, /* MOVE */
{ op_3188, 0, 12680 }, /* MOVE */
{ op_3190, 0, 12688 }, /* MOVE */
{ op_3198, 0, 12696 }, /* MOVE */
{ op_31a0, 0, 12704 }, /* MOVE */
{ op_31a8, 0, 12712 }, /* MOVE */
{ op_31b0, 0, 12720 }, /* MOVE */
{ op_31b8, 0, 12728 }, /* MOVE */
{ op_31b9, 0, 12729 }, /* MOVE */
{ op_31ba, 0, 12730 }, /* MOVE */
{ op_31bb, 0, 12731 }, /* MOVE */
{ op_31bc, 0, 12732 }, /* MOVE */
{ op_31c0, 0, 12736 }, /* MOVE */
{ op_31c8, 0, 12744 }, /* MOVE */
{ op_31d0, 0, 12752 }, /* MOVE */
{ op_31d8, 0, 12760 }, /* MOVE */
{ op_31e0, 0, 12768 }, /* MOVE */
{ op_31e8, 0, 12776 }, /* MOVE */
{ op_31f0, 0, 12784 }, /* MOVE */
{ op_31f8, 0, 12792 }, /* MOVE */
{ op_31f9, 0, 12793 }, /* MOVE */
{ op_31fa, 0, 12794 }, /* MOVE */
{ op_31fb, 0, 12795 }, /* MOVE */
{ op_31fc, 0, 12796 }, /* MOVE */
{ op_33c0, 0, 13248 }, /* MOVE */
{ op_33c8, 0, 13256 }, /* MOVE */
{ op_33d0, 0, 13264 }, /* MOVE */
{ op_33d8, 0, 13272 }, /* MOVE */
{ op_33e0, 0, 13280 }, /* MOVE */
{ op_33e8, 0, 13288 }, /* MOVE */
{ op_33f0, 0, 13296 }, /* MOVE */
{ op_33f8, 0, 13304 }, /* MOVE */
{ op_33f9, 0, 13305 }, /* MOVE */
{ op_33fa, 0, 13306 }, /* MOVE */
{ op_33fb, 0, 13307 }, /* MOVE */
{ op_33fc, 0, 13308 }, /* MOVE */
{ op_4000, 0, 16384 }, /* NEGX */
{ op_4010, 0, 16400 }, /* NEGX */
{ op_4018, 0, 16408 }, /* NEGX */
{ op_4020, 0, 16416 }, /* NEGX */
{ op_4028, 0, 16424 }, /* NEGX */
{ op_4030, 0, 16432 }, /* NEGX */
{ op_4038, 0, 16440 }, /* NEGX */
{ op_4039, 0, 16441 }, /* NEGX */
{ op_4040, 0, 16448 }, /* NEGX */
{ op_4050, 0, 16464 }, /* NEGX */
{ op_4058, 0, 16472 }, /* NEGX */
{ op_4060, 0, 16480 }, /* NEGX */
{ op_4068, 0, 16488 }, /* NEGX */
{ op_4070, 0, 16496 }, /* NEGX */
{ op_4078, 0, 16504 }, /* NEGX */
{ op_4079, 0, 16505 }, /* NEGX */
{ op_4080, 0, 16512 }, /* NEGX */
{ op_4090, 0, 16528 }, /* NEGX */
{ op_4098, 0, 16536 }, /* NEGX */
{ op_40a0, 0, 16544 }, /* NEGX */
{ op_40a8, 0, 16552 }, /* NEGX */
{ op_40b0, 0, 16560 }, /* NEGX */
{ op_40b8, 0, 16568 }, /* NEGX */
{ op_40b9, 0, 16569 }, /* NEGX */
{ op_40c0, 0, 16576 }, /* MVSR2 */
{ op_40d0, 0, 16592 }, /* MVSR2 */
{ op_40d8, 0, 16600 }, /* MVSR2 */
{ op_40e0, 0, 16608 }, /* MVSR2 */
{ op_40e8, 0, 16616 }, /* MVSR2 */
{ op_40f0, 0, 16624 }, /* MVSR2 */
{ op_40f8, 0, 16632 }, /* MVSR2 */
{ op_40f9, 0, 16633 }, /* MVSR2 */
{ op_4100, 0, 16640 }, /* CHK */
{ op_4110, 0, 16656 }, /* CHK */
{ op_4118, 0, 16664 }, /* CHK */
{ op_4120, 0, 16672 }, /* CHK */
{ op_4128, 0, 16680 }, /* CHK */
{ op_4130, 0, 16688 }, /* CHK */
{ op_4138, 0, 16696 }, /* CHK */
{ op_4139, 0, 16697 }, /* CHK */
{ op_413a, 0, 16698 }, /* CHK */
{ op_413b, 0, 16699 }, /* CHK */
{ op_413c, 0, 16700 }, /* CHK */
{ op_4180, 0, 16768 }, /* CHK */
{ op_4190, 0, 16784 }, /* CHK */
{ op_4198, 0, 16792 }, /* CHK */
{ op_41a0, 0, 16800 }, /* CHK */
{ op_41a8, 0, 16808 }, /* CHK */
{ op_41b0, 0, 16816 }, /* CHK */
{ op_41b8, 0, 16824 }, /* CHK */
{ op_41b9, 0, 16825 }, /* CHK */
{ op_41ba, 0, 16826 }, /* CHK */
{ op_41bb, 0, 16827 }, /* CHK */
{ op_41bc, 0, 16828 }, /* CHK */
{ op_41d0, 0, 16848 }, /* LEA */
{ op_41e8, 0, 16872 }, /* LEA */
{ op_41f0, 0, 16880 }, /* LEA */
{ op_41f8, 0, 16888 }, /* LEA */
{ op_41f9, 0, 16889 }, /* LEA */
{ op_41fa, 0, 16890 }, /* LEA */
{ op_41fb, 0, 16891 }, /* LEA */
{ op_4200, 0, 16896 }, /* CLR */
{ op_4210, 0, 16912 }, /* CLR */
{ op_4218, 0, 16920 }, /* CLR */
{ op_4220, 0, 16928 }, /* CLR */
{ op_4228, 0, 16936 }, /* CLR */
{ op_4230, 0, 16944 }, /* CLR */
{ op_4238, 0, 16952 }, /* CLR */
{ op_4239, 0, 16953 }, /* CLR */
{ op_4240, 0, 16960 }, /* CLR */
{ op_4250, 0, 16976 }, /* CLR */
{ op_4258, 0, 16984 }, /* CLR */
{ op_4260, 0, 16992 }, /* CLR */
{ op_4268, 0, 17000 }, /* CLR */
{ op_4270, 0, 17008 }, /* CLR */
{ op_4278, 0, 17016 }, /* CLR */
{ op_4279, 0, 17017 }, /* CLR */
{ op_4280, 0, 17024 }, /* CLR */
{ op_4290, 0, 17040 }, /* CLR */
{ op_4298, 0, 17048 }, /* CLR */
{ op_42a0, 0, 17056 }, /* CLR */
{ op_42a8, 0, 17064 }, /* CLR */
{ op_42b0, 0, 17072 }, /* CLR */
{ op_42b8, 0, 17080 }, /* CLR */
{ op_42b9, 0, 17081 }, /* CLR */
{ op_4400, 0, 17408 }, /* NEG */
{ op_4410, 0, 17424 }, /* NEG */
{ op_4418, 0, 17432 }, /* NEG */
{ op_4420, 0, 17440 }, /* NEG */
{ op_4428, 0, 17448 }, /* NEG */
{ op_4430, 0, 17456 }, /* NEG */
{ op_4438, 0, 17464 }, /* NEG */
{ op_4439, 0, 17465 }, /* NEG */
{ op_4440, 0, 17472 }, /* NEG */
{ op_4450, 0, 17488 }, /* NEG */
{ op_4458, 0, 17496 }, /* NEG */
{ op_4460, 0, 17504 }, /* NEG */
{ op_4468, 0, 17512 }, /* NEG */
{ op_4470, 0, 17520 }, /* NEG */
{ op_4478, 0, 17528 }, /* NEG */
{ op_4479, 0, 17529 }, /* NEG */
{ op_4480, 0, 17536 }, /* NEG */
{ op_4490, 0, 17552 }, /* NEG */
{ op_4498, 0, 17560 }, /* NEG */
{ op_44a0, 0, 17568 }, /* NEG */
{ op_44a8, 0, 17576 }, /* NEG */
{ op_44b0, 0, 17584 }, /* NEG */
{ op_44b8, 0, 17592 }, /* NEG */
{ op_44b9, 0, 17593 }, /* NEG */
{ op_44c0, 0, 17600 }, /* MV2SR */
{ op_44d0, 0, 17616 }, /* MV2SR */
{ op_44d8, 0, 17624 }, /* MV2SR */
{ op_44e0, 0, 17632 }, /* MV2SR */
{ op_44e8, 0, 17640 }, /* MV2SR */
{ op_44f0, 0, 17648 }, /* MV2SR */
{ op_44f8, 0, 17656 }, /* MV2SR */
{ op_44f9, 0, 17657 }, /* MV2SR */
{ op_44fa, 0, 17658 }, /* MV2SR */
{ op_44fb, 0, 17659 }, /* MV2SR */
{ op_44fc, 0, 17660 }, /* MV2SR */
{ op_4600, 0, 17920 }, /* NOT */
{ op_4610, 0, 17936 }, /* NOT */
{ op_4618, 0, 17944 }, /* NOT */
{ op_4620, 0, 17952 }, /* NOT */
{ op_4628, 0, 17960 }, /* NOT */
{ op_4630, 0, 17968 }, /* NOT */
{ op_4638, 0, 17976 }, /* NOT */
{ op_4639, 0, 17977 }, /* NOT */
{ op_4640, 0, 17984 }, /* NOT */
{ op_4650, 0, 18000 }, /* NOT */
{ op_4658, 0, 18008 }, /* NOT */
{ op_4660, 0, 18016 }, /* NOT */
{ op_4668, 0, 18024 }, /* NOT */
{ op_4670, 0, 18032 }, /* NOT */
{ op_4678, 0, 18040 }, /* NOT */
{ op_4679, 0, 18041 }, /* NOT */
{ op_4680, 0, 18048 }, /* NOT */
{ op_4690, 0, 18064 }, /* NOT */
{ op_4698, 0, 18072 }, /* NOT */
{ op_46a0, 0, 18080 }, /* NOT */
{ op_46a8, 0, 18088 }, /* NOT */
{ op_46b0, 0, 18096 }, /* NOT */
{ op_46b8, 0, 18104 }, /* NOT */
{ op_46b9, 0, 18105 }, /* NOT */
{ op_46c0, 0, 18112 }, /* MV2SR */
{ op_46d0, 0, 18128 }, /* MV2SR */
{ op_46d8, 0, 18136 }, /* MV2SR */
{ op_46e0, 0, 18144 }, /* MV2SR */
{ op_46e8, 0, 18152 }, /* MV2SR */
{ op_46f0, 0, 18160 }, /* MV2SR */
{ op_46f8, 0, 18168 }, /* MV2SR */
{ op_46f9, 0, 18169 }, /* MV2SR */
{ op_46fa, 0, 18170 }, /* MV2SR */
{ op_46fb, 0, 18171 }, /* MV2SR */
{ op_46fc, 0, 18172 }, /* MV2SR */
{ op_4800, 0, 18432 }, /* NBCD */
{ op_4810, 0, 18448 }, /* NBCD */
{ op_4818, 0, 18456 }, /* NBCD */
{ op_4820, 0, 18464 }, /* NBCD */
{ op_4828, 0, 18472 }, /* NBCD */
{ op_4830, 0, 18480 }, /* NBCD */
{ op_4838, 0, 18488 }, /* NBCD */
{ op_4839, 0, 18489 }, /* NBCD */
{ op_4840, 0, 18496 }, /* SWAP */
{ op_4850, 0, 18512 }, /* PEA */
{ op_4868, 0, 18536 }, /* PEA */
{ op_4870, 0, 18544 }, /* PEA */
{ op_4878, 0, 18552 }, /* PEA */
{ op_4879, 0, 18553 }, /* PEA */
{ op_487a, 0, 18554 }, /* PEA */
{ op_487b, 0, 18555 }, /* PEA */
{ op_4880, 0, 18560 }, /* EXT */
{ op_4890, 0, 18576 }, /* MVMLE */
{ op_48a0, 0, 18592 }, /* MVMLE */
{ op_48a8, 0, 18600 }, /* MVMLE */
{ op_48b0, 0, 18608 }, /* MVMLE */
{ op_48b8, 0, 18616 }, /* MVMLE */
{ op_48b9, 0, 18617 }, /* MVMLE */
{ op_48c0, 0, 18624 }, /* EXT */
{ op_48d0, 0, 18640 }, /* MVMLE */
{ op_48e0, 0, 18656 }, /* MVMLE */
{ op_48e8, 0, 18664 }, /* MVMLE */
{ op_48f0, 0, 18672 }, /* MVMLE */
{ op_48f8, 0, 18680 }, /* MVMLE */
{ op_48f9, 0, 18681 }, /* MVMLE */
{ op_49c0, 0, 18880 }, /* EXT */
{ op_4a00, 0, 18944 }, /* TST */
{ op_4a10, 0, 18960 }, /* TST */
{ op_4a18, 0, 18968 }, /* TST */
{ op_4a20, 0, 18976 }, /* TST */
{ op_4a28, 0, 18984 }, /* TST */
{ op_4a30, 0, 18992 }, /* TST */
{ op_4a38, 0, 19000 }, /* TST */
{ op_4a39, 0, 19001 }, /* TST */
{ op_4a3a, 0, 19002 }, /* TST */
{ op_4a3b, 0, 19003 }, /* TST */
{ op_4a3c, 0, 19004 }, /* TST */
{ op_4a40, 0, 19008 }, /* TST */
{ op_4a48, 0, 19016 }, /* TST */
{ op_4a50, 0, 19024 }, /* TST */
{ op_4a58, 0, 19032 }, /* TST */
{ op_4a60, 0, 19040 }, /* TST */
{ op_4a68, 0, 19048 }, /* TST */
{ op_4a70, 0, 19056 }, /* TST */
{ op_4a78, 0, 19064 }, /* TST */
{ op_4a79, 0, 19065 }, /* TST */
{ op_4a7a, 0, 19066 }, /* TST */
{ op_4a7b, 0, 19067 }, /* TST */
{ op_4a7c, 0, 19068 }, /* TST */
{ op_4a80, 0, 19072 }, /* TST */
{ op_4a88, 0, 19080 }, /* TST */
{ op_4a90, 0, 19088 }, /* TST */
{ op_4a98, 0, 19096 }, /* TST */
{ op_4aa0, 0, 19104 }, /* TST */
{ op_4aa8, 0, 19112 }, /* TST */
{ op_4ab0, 0, 19120 }, /* TST */
{ op_4ab8, 0, 19128 }, /* TST */
{ op_4ab9, 0, 19129 }, /* TST */
{ op_4aba, 0, 19130 }, /* TST */
{ op_4abb, 0, 19131 }, /* TST */
{ op_4abc, 0, 19132 }, /* TST */
{ op_4ac0, 0, 19136 }, /* TAS */
{ op_4ad0, 0, 19152 }, /* TAS */
{ op_4ad8, 0, 19160 }, /* TAS */
{ op_4ae0, 0, 19168 }, /* TAS */
{ op_4ae8, 0, 19176 }, /* TAS */
{ op_4af0, 0, 19184 }, /* TAS */
{ op_4af8, 0, 19192 }, /* TAS */
{ op_4af9, 0, 19193 }, /* TAS */
{ op_4c90, 0, 19600 }, /* MVMEL */
{ op_4c98, 0, 19608 }, /* MVMEL */
{ op_4ca8, 0, 19624 }, /* MVMEL */
{ op_4cb0, 0, 19632 }, /* MVMEL */
{ op_4cb8, 0, 19640 }, /* MVMEL */
{ op_4cb9, 0, 19641 }, /* MVMEL */
{ op_4cba, 0, 19642 }, /* MVMEL */
{ op_4cbb, 0, 19643 }, /* MVMEL */
{ op_4cd0, 0, 19664 }, /* MVMEL */
{ op_4cd8, 0, 19672 }, /* MVMEL */
{ op_4ce8, 0, 19688 }, /* MVMEL */
{ op_4cf0, 0, 19696 }, /* MVMEL */
{ op_4cf8, 0, 19704 }, /* MVMEL */
{ op_4cf9, 0, 19705 }, /* MVMEL */
{ op_4cfa, 0, 19706 }, /* MVMEL */
{ op_4cfb, 0, 19707 }, /* MVMEL */
{ op_4e40, 0, 20032 }, /* TRAP */
{ op_4e50, 0, 20048 }, /* LINK */
{ op_4e58, 0, 20056 }, /* UNLK */
{ op_4e60, 0, 20064 }, /* MVR2USP */
{ op_4e68, 0, 20072 }, /* MVUSP2R */
{ op_4e70, 0, 20080 }, /* RESET */
{ op_4e71, 0, 20081 }, /* NOP */
{ op_4e72, 0, 20082 }, /* STOP */
{ op_4e73, 0, 20083 }, /* RTE */
{ op_4e74, 0, 20084 }, /* RTD */
{ op_4e75, 0, 20085 }, /* RTS */
{ op_4e76, 0, 20086 }, /* TRAPV */
{ op_4e77, 0, 20087 }, /* RTR */
{ op_4e90, 0, 20112 }, /* JSR */
{ op_4ea8, 0, 20136 }, /* JSR */
{ op_4eb0, 0, 20144 }, /* JSR */
{ op_4eb8, 0, 20152 }, /* JSR */
{ op_4eb9, 0, 20153 }, /* JSR */
{ op_4eba, 0, 20154 }, /* JSR */
{ op_4ebb, 0, 20155 }, /* JSR */
{ op_4ed0, 0, 20176 }, /* JMP */
{ op_4ee8, 0, 20200 }, /* JMP */
{ op_4ef0, 0, 20208 }, /* JMP */
{ op_4ef8, 0, 20216 }, /* JMP */
{ op_4ef9, 0, 20217 }, /* JMP */
{ op_4efa, 0, 20218 }, /* JMP */
{ op_4efb, 0, 20219 }, /* JMP */
{ op_5000, 0, 20480 }, /* ADD */
{ op_5010, 0, 20496 }, /* ADD */
{ op_5018, 0, 20504 }, /* ADD */
{ op_5020, 0, 20512 }, /* ADD */
{ op_5028, 0, 20520 }, /* ADD */
{ op_5030, 0, 20528 }, /* ADD */
{ op_5038, 0, 20536 }, /* ADD */
{ op_5039, 0, 20537 }, /* ADD */
{ op_5040, 0, 20544 }, /* ADD */
{ op_5048, 0, 20552 }, /* ADDA */
{ op_5050, 0, 20560 }, /* ADD */
{ op_5058, 0, 20568 }, /* ADD */
{ op_5060, 0, 20576 }, /* ADD */
{ op_5068, 0, 20584 }, /* ADD */
{ op_5070, 0, 20592 }, /* ADD */
{ op_5078, 0, 20600 }, /* ADD */
{ op_5079, 0, 20601 }, /* ADD */
{ op_5080, 0, 20608 }, /* ADD */
{ op_5088, 0, 20616 }, /* ADDA */
{ op_5090, 0, 20624 }, /* ADD */
{ op_5098, 0, 20632 }, /* ADD */
{ op_50a0, 0, 20640 }, /* ADD */
{ op_50a8, 0, 20648 }, /* ADD */
{ op_50b0, 0, 20656 }, /* ADD */
{ op_50b8, 0, 20664 }, /* ADD */
{ op_50b9, 0, 20665 }, /* ADD */
{ op_50c0, 0, 20672 }, /* Scc */
{ op_50c8, 0, 20680 }, /* DBcc */
{ op_50d0, 0, 20688 }, /* Scc */
{ op_50d8, 0, 20696 }, /* Scc */
{ op_50e0, 0, 20704 }, /* Scc */
{ op_50e8, 0, 20712 }, /* Scc */
{ op_50f0, 0, 20720 }, /* Scc */
{ op_50f8, 0, 20728 }, /* Scc */
{ op_50f9, 0, 20729 }, /* Scc */
{ op_5100, 0, 20736 }, /* SUB */
{ op_5110, 0, 20752 }, /* SUB */
{ op_5118, 0, 20760 }, /* SUB */
{ op_5120, 0, 20768 }, /* SUB */
{ op_5128, 0, 20776 }, /* SUB */
{ op_5130, 0, 20784 }, /* SUB */
{ op_5138, 0, 20792 }, /* SUB */
{ op_5139, 0, 20793 }, /* SUB */
{ op_5140, 0, 20800 }, /* SUB */
{ op_5148, 0, 20808 }, /* SUBA */
{ op_5150, 0, 20816 }, /* SUB */
{ op_5158, 0, 20824 }, /* SUB */
{ op_5160, 0, 20832 }, /* SUB */
{ op_5168, 0, 20840 }, /* SUB */
{ op_5170, 0, 20848 }, /* SUB */
{ op_5178, 0, 20856 }, /* SUB */
{ op_5179, 0, 20857 }, /* SUB */
{ op_5180, 0, 20864 }, /* SUB */
{ op_5188, 0, 20872 }, /* SUBA */
{ op_5190, 0, 20880 }, /* SUB */
{ op_5198, 0, 20888 }, /* SUB */
{ op_51a0, 0, 20896 }, /* SUB */
{ op_51a8, 0, 20904 }, /* SUB */
{ op_51b0, 0, 20912 }, /* SUB */
{ op_51b8, 0, 20920 }, /* SUB */
{ op_51b9, 0, 20921 }, /* SUB */
{ op_51c0, 0, 20928 }, /* Scc */
{ op_51c8, 0, 20936 }, /* DBcc */
{ op_51d0, 0, 20944 }, /* Scc */
{ op_51d8, 0, 20952 }, /* Scc */
{ op_51e0, 0, 20960 }, /* Scc */
{ op_51e8, 0, 20968 }, /* Scc */
{ op_51f0, 0, 20976 }, /* Scc */
{ op_51f8, 0, 20984 }, /* Scc */
{ op_51f9, 0, 20985 }, /* Scc */
{ op_52c0, 0, 21184 }, /* Scc */
{ op_52c8, 0, 21192 }, /* DBcc */
{ op_52d0, 0, 21200 }, /* Scc */
{ op_52d8, 0, 21208 }, /* Scc */
{ op_52e0, 0, 21216 }, /* Scc */
{ op_52e8, 0, 21224 }, /* Scc */
{ op_52f0, 0, 21232 }, /* Scc */
{ op_52f8, 0, 21240 }, /* Scc */
{ op_52f9, 0, 21241 }, /* Scc */
{ op_53c0, 0, 21440 }, /* Scc */
{ op_53c8, 0, 21448 }, /* DBcc */
{ op_53d0, 0, 21456 }, /* Scc */
{ op_53d8, 0, 21464 }, /* Scc */
{ op_53e0, 0, 21472 }, /* Scc */
{ op_53e8, 0, 21480 }, /* Scc */
{ op_53f0, 0, 21488 }, /* Scc */
{ op_53f8, 0, 21496 }, /* Scc */
{ op_53f9, 0, 21497 }, /* Scc */
{ op_54c0, 0, 21696 }, /* Scc */
{ op_54c8, 0, 21704 }, /* DBcc */
{ op_54d0, 0, 21712 }, /* Scc */
{ op_54d8, 0, 21720 }, /* Scc */
{ op_54e0, 0, 21728 }, /* Scc */
{ op_54e8, 0, 21736 }, /* Scc */
{ op_54f0, 0, 21744 }, /* Scc */
{ op_54f8, 0, 21752 }, /* Scc */
{ op_54f9, 0, 21753 }, /* Scc */
{ op_55c0, 0, 21952 }, /* Scc */
{ op_55c8, 0, 21960 }, /* DBcc */
{ op_55d0, 0, 21968 }, /* Scc */
{ op_55d8, 0, 21976 }, /* Scc */
{ op_55e0, 0, 21984 }, /* Scc */
{ op_55e8, 0, 21992 }, /* Scc */
{ op_55f0, 0, 22000 }, /* Scc */
{ op_55f8, 0, 22008 }, /* Scc */
{ op_55f9, 0, 22009 }, /* Scc */
{ op_56c0, 0, 22208 }, /* Scc */
{ op_56c8, 0, 22216 }, /* DBcc */
{ op_56d0, 0, 22224 }, /* Scc */
{ op_56d8, 0, 22232 }, /* Scc */
{ op_56e0, 0, 22240 }, /* Scc */
{ op_56e8, 0, 22248 }, /* Scc */
{ op_56f0, 0, 22256 }, /* Scc */
{ op_56f8, 0, 22264 }, /* Scc */
{ op_56f9, 0, 22265 }, /* Scc */
{ op_57c0, 0, 22464 }, /* Scc */
{ op_57c8, 0, 22472 }, /* DBcc */
{ op_57d0, 0, 22480 }, /* Scc */
{ op_57d8, 0, 22488 }, /* Scc */
{ op_57e0, 0, 22496 }, /* Scc */
{ op_57e8, 0, 22504 }, /* Scc */
{ op_57f0, 0, 22512 }, /* Scc */
{ op_57f8, 0, 22520 }, /* Scc */
{ op_57f9, 0, 22521 }, /* Scc */
{ op_58c0, 0, 22720 }, /* Scc */
{ op_58c8, 0, 22728 }, /* DBcc */
{ op_58d0, 0, 22736 }, /* Scc */
{ op_58d8, 0, 22744 }, /* Scc */
{ op_58e0, 0, 22752 }, /* Scc */
{ op_58e8, 0, 22760 }, /* Scc */
{ op_58f0, 0, 22768 }, /* Scc */
{ op_58f8, 0, 22776 }, /* Scc */
{ op_58f9, 0, 22777 }, /* Scc */
{ op_59c0, 0, 22976 }, /* Scc */
{ op_59c8, 0, 22984 }, /* DBcc */
{ op_59d0, 0, 22992 }, /* Scc */
{ op_59d8, 0, 23000 }, /* Scc */
{ op_59e0, 0, 23008 }, /* Scc */
{ op_59e8, 0, 23016 }, /* Scc */
{ op_59f0, 0, 23024 }, /* Scc */
{ op_59f8, 0, 23032 }, /* Scc */
{ op_59f9, 0, 23033 }, /* Scc */
{ op_5ac0, 0, 23232 }, /* Scc */
{ op_5ac8, 0, 23240 }, /* DBcc */
{ op_5ad0, 0, 23248 }, /* Scc */
{ op_5ad8, 0, 23256 }, /* Scc */
{ op_5ae0, 0, 23264 }, /* Scc */
{ op_5ae8, 0, 23272 }, /* Scc */
{ op_5af0, 0, 23280 }, /* Scc */
{ op_5af8, 0, 23288 }, /* Scc */
{ op_5af9, 0, 23289 }, /* Scc */
{ op_5bc0, 0, 23488 }, /* Scc */
{ op_5bc8, 0, 23496 }, /* DBcc */
{ op_5bd0, 0, 23504 }, /* Scc */
{ op_5bd8, 0, 23512 }, /* Scc */
{ op_5be0, 0, 23520 }, /* Scc */
{ op_5be8, 0, 23528 }, /* Scc */
{ op_5bf0, 0, 23536 }, /* Scc */
{ op_5bf8, 0, 23544 }, /* Scc */
{ op_5bf9, 0, 23545 }, /* Scc */
{ op_5cc0, 0, 23744 }, /* Scc */
{ op_5cc8, 0, 23752 }, /* DBcc */
{ op_5cd0, 0, 23760 }, /* Scc */
{ op_5cd8, 0, 23768 }, /* Scc */
{ op_5ce0, 0, 23776 }, /* Scc */
{ op_5ce8, 0, 23784 }, /* Scc */
{ op_5cf0, 0, 23792 }, /* Scc */
{ op_5cf8, 0, 23800 }, /* Scc */
{ op_5cf9, 0, 23801 }, /* Scc */
{ op_5dc0, 0, 24000 }, /* Scc */
{ op_5dc8, 0, 24008 }, /* DBcc */
{ op_5dd0, 0, 24016 }, /* Scc */
{ op_5dd8, 0, 24024 }, /* Scc */
{ op_5de0, 0, 24032 }, /* Scc */
{ op_5de8, 0, 24040 }, /* Scc */
{ op_5df0, 0, 24048 }, /* Scc */
{ op_5df8, 0, 24056 }, /* Scc */
{ op_5df9, 0, 24057 }, /* Scc */
{ op_5ec0, 0, 24256 }, /* Scc */
{ op_5ec8, 0, 24264 }, /* DBcc */
{ op_5ed0, 0, 24272 }, /* Scc */
{ op_5ed8, 0, 24280 }, /* Scc */
{ op_5ee0, 0, 24288 }, /* Scc */
{ op_5ee8, 0, 24296 }, /* Scc */
{ op_5ef0, 0, 24304 }, /* Scc */
{ op_5ef8, 0, 24312 }, /* Scc */
{ op_5ef9, 0, 24313 }, /* Scc */
{ op_5fc0, 0, 24512 }, /* Scc */
{ op_5fc8, 0, 24520 }, /* DBcc */
{ op_5fd0, 0, 24528 }, /* Scc */
{ op_5fd8, 0, 24536 }, /* Scc */
{ op_5fe0, 0, 24544 }, /* Scc */
{ op_5fe8, 0, 24552 }, /* Scc */
{ op_5ff0, 0, 24560 }, /* Scc */
{ op_5ff8, 0, 24568 }, /* Scc */
{ op_5ff9, 0, 24569 }, /* Scc */
{ op_6000, 0, 24576 }, /* Bcc */
{ op_6001, 0, 24577 }, /* Bcc */
{ op_60ff, 0, 24831 }, /* Bcc */
{ op_6100, 0, 24832 }, /* BSR */
{ op_6101, 0, 24833 }, /* BSR */
{ op_61ff, 0, 25087 }, /* BSR */
{ op_6200, 0, 25088 }, /* Bcc */
{ op_6201, 0, 25089 }, /* Bcc */
{ op_62ff, 0, 25343 }, /* Bcc */
{ op_6300, 0, 25344 }, /* Bcc */
{ op_6301, 0, 25345 }, /* Bcc */
{ op_63ff, 0, 25599 }, /* Bcc */
{ op_6400, 0, 25600 }, /* Bcc */
{ op_6401, 0, 25601 }, /* Bcc */
{ op_64ff, 0, 25855 }, /* Bcc */
{ op_6500, 0, 25856 }, /* Bcc */
{ op_6501, 0, 25857 }, /* Bcc */
{ op_65ff, 0, 26111 }, /* Bcc */
{ op_6600, 0, 26112 }, /* Bcc */
{ op_6601, 0, 26113 }, /* Bcc */
{ op_66ff, 0, 26367 }, /* Bcc */
{ op_6700, 0, 26368 }, /* Bcc */
{ op_6701, 0, 26369 }, /* Bcc */
{ op_67ff, 0, 26623 }, /* Bcc */
{ op_6800, 0, 26624 }, /* Bcc */
{ op_6801, 0, 26625 }, /* Bcc */
{ op_68ff, 0, 26879 }, /* Bcc */
{ op_6900, 0, 26880 }, /* Bcc */
{ op_6901, 0, 26881 }, /* Bcc */
{ op_69ff, 0, 27135 }, /* Bcc */
{ op_6a00, 0, 27136 }, /* Bcc */
{ op_6a01, 0, 27137 }, /* Bcc */
{ op_6aff, 0, 27391 }, /* Bcc */
{ op_6b00, 0, 27392 }, /* Bcc */
{ op_6b01, 0, 27393 }, /* Bcc */
{ op_6bff, 0, 27647 }, /* Bcc */
{ op_6c00, 0, 27648 }, /* Bcc */
{ op_6c01, 0, 27649 }, /* Bcc */
{ op_6cff, 0, 27903 }, /* Bcc */
{ op_6d00, 0, 27904 }, /* Bcc */
{ op_6d01, 0, 27905 }, /* Bcc */
{ op_6dff, 0, 28159 }, /* Bcc */
{ op_6e00, 0, 28160 }, /* Bcc */
{ op_6e01, 0, 28161 }, /* Bcc */
{ op_6eff, 0, 28415 }, /* Bcc */
{ op_6f00, 0, 28416 }, /* Bcc */
{ op_6f01, 0, 28417 }, /* Bcc */
{ op_6fff, 0, 28671 }, /* Bcc */
{ op_7000, 0, 28672 }, /* MOVE */
{ op_8000, 0, 32768 }, /* OR */
{ op_8010, 0, 32784 }, /* OR */
{ op_8018, 0, 32792 }, /* OR */
{ op_8020, 0, 32800 }, /* OR */
{ op_8028, 0, 32808 }, /* OR */
{ op_8030, 0, 32816 }, /* OR */
{ op_8038, 0, 32824 }, /* OR */
{ op_8039, 0, 32825 }, /* OR */
{ op_803a, 0, 32826 }, /* OR */
{ op_803b, 0, 32827 }, /* OR */
{ op_803c, 0, 32828 }, /* OR */
{ op_8040, 0, 32832 }, /* OR */
{ op_8050, 0, 32848 }, /* OR */
{ op_8058, 0, 32856 }, /* OR */
{ op_8060, 0, 32864 }, /* OR */
{ op_8068, 0, 32872 }, /* OR */
{ op_8070, 0, 32880 }, /* OR */
{ op_8078, 0, 32888 }, /* OR */
{ op_8079, 0, 32889 }, /* OR */
{ op_807a, 0, 32890 }, /* OR */
{ op_807b, 0, 32891 }, /* OR */
{ op_807c, 0, 32892 }, /* OR */
{ op_8080, 0, 32896 }, /* OR */
{ op_8090, 0, 32912 }, /* OR */
{ op_8098, 0, 32920 }, /* OR */
{ op_80a0, 0, 32928 }, /* OR */
{ op_80a8, 0, 32936 }, /* OR */
{ op_80b0, 0, 32944 }, /* OR */
{ op_80b8, 0, 32952 }, /* OR */
{ op_80b9, 0, 32953 }, /* OR */
{ op_80ba, 0, 32954 }, /* OR */
{ op_80bb, 0, 32955 }, /* OR */
{ op_80bc, 0, 32956 }, /* OR */
{ op_80c0, 0, 32960 }, /* DIVU */
{ op_80d0, 0, 32976 }, /* DIVU */
{ op_80d8, 0, 32984 }, /* DIVU */
{ op_80e0, 0, 32992 }, /* DIVU */
{ op_80e8, 0, 33000 }, /* DIVU */
{ op_80f0, 0, 33008 }, /* DIVU */
{ op_80f8, 0, 33016 }, /* DIVU */
{ op_80f9, 0, 33017 }, /* DIVU */
{ op_80fa, 0, 33018 }, /* DIVU */
{ op_80fb, 0, 33019 }, /* DIVU */
{ op_80fc, 0, 33020 }, /* DIVU */
{ op_8100, 0, 33024 }, /* SBCD */
{ op_8108, 0, 33032 }, /* SBCD */
{ op_8110, 0, 33040 }, /* OR */
{ op_8118, 0, 33048 }, /* OR */
{ op_8120, 0, 33056 }, /* OR */
{ op_8128, 0, 33064 }, /* OR */
{ op_8130, 0, 33072 }, /* OR */
{ op_8138, 0, 33080 }, /* OR */
{ op_8139, 0, 33081 }, /* OR */
{ op_8150, 0, 33104 }, /* OR */
{ op_8158, 0, 33112 }, /* OR */
{ op_8160, 0, 33120 }, /* OR */
{ op_8168, 0, 33128 }, /* OR */
{ op_8170, 0, 33136 }, /* OR */
{ op_8178, 0, 33144 }, /* OR */
{ op_8179, 0, 33145 }, /* OR */
{ op_8190, 0, 33168 }, /* OR */
{ op_8198, 0, 33176 }, /* OR */
{ op_81a0, 0, 33184 }, /* OR */
{ op_81a8, 0, 33192 }, /* OR */
{ op_81b0, 0, 33200 }, /* OR */
{ op_81b8, 0, 33208 }, /* OR */
{ op_81b9, 0, 33209 }, /* OR */
{ op_81c0, 0, 33216 }, /* DIVS */
{ op_81d0, 0, 33232 }, /* DIVS */
{ op_81d8, 0, 33240 }, /* DIVS */
{ op_81e0, 0, 33248 }, /* DIVS */
{ op_81e8, 0, 33256 }, /* DIVS */
{ op_81f0, 0, 33264 }, /* DIVS */
{ op_81f8, 0, 33272 }, /* DIVS */
{ op_81f9, 0, 33273 }, /* DIVS */
{ op_81fa, 0, 33274 }, /* DIVS */
{ op_81fb, 0, 33275 }, /* DIVS */
{ op_81fc, 0, 33276 }, /* DIVS */
{ op_9000, 0, 36864 }, /* SUB */
{ op_9010, 0, 36880 }, /* SUB */
{ op_9018, 0, 36888 }, /* SUB */
{ op_9020, 0, 36896 }, /* SUB */
{ op_9028, 0, 36904 }, /* SUB */
{ op_9030, 0, 36912 }, /* SUB */
{ op_9038, 0, 36920 }, /* SUB */
{ op_9039, 0, 36921 }, /* SUB */
{ op_903a, 0, 36922 }, /* SUB */
{ op_903b, 0, 36923 }, /* SUB */
{ op_903c, 0, 36924 }, /* SUB */
{ op_9040, 0, 36928 }, /* SUB */
{ op_9048, 0, 36936 }, /* SUB */
{ op_9050, 0, 36944 }, /* SUB */
{ op_9058, 0, 36952 }, /* SUB */
{ op_9060, 0, 36960 }, /* SUB */
{ op_9068, 0, 36968 }, /* SUB */
{ op_9070, 0, 36976 }, /* SUB */
{ op_9078, 0, 36984 }, /* SUB */
{ op_9079, 0, 36985 }, /* SUB */
{ op_907a, 0, 36986 }, /* SUB */
{ op_907b, 0, 36987 }, /* SUB */
{ op_907c, 0, 36988 }, /* SUB */
{ op_9080, 0, 36992 }, /* SUB */
{ op_9088, 0, 37000 }, /* SUB */
{ op_9090, 0, 37008 }, /* SUB */
{ op_9098, 0, 37016 }, /* SUB */
{ op_90a0, 0, 37024 }, /* SUB */
{ op_90a8, 0, 37032 }, /* SUB */
{ op_90b0, 0, 37040 }, /* SUB */
{ op_90b8, 0, 37048 }, /* SUB */
{ op_90b9, 0, 37049 }, /* SUB */
{ op_90ba, 0, 37050 }, /* SUB */
{ op_90bb, 0, 37051 }, /* SUB */
{ op_90bc, 0, 37052 }, /* SUB */
{ op_90c0, 0, 37056 }, /* SUBA */
{ op_90c8, 0, 37064 }, /* SUBA */
{ op_90d0, 0, 37072 }, /* SUBA */
{ op_90d8, 0, 37080 }, /* SUBA */
{ op_90e0, 0, 37088 }, /* SUBA */
{ op_90e8, 0, 37096 }, /* SUBA */
{ op_90f0, 0, 37104 }, /* SUBA */
{ op_90f8, 0, 37112 }, /* SUBA */
{ op_90f9, 0, 37113 }, /* SUBA */
{ op_90fa, 0, 37114 }, /* SUBA */
{ op_90fb, 0, 37115 }, /* SUBA */
{ op_90fc, 0, 37116 }, /* SUBA */
{ op_9100, 0, 37120 }, /* SUBX */
{ op_9108, 0, 37128 }, /* SUBX */
{ op_9110, 0, 37136 }, /* SUB */
{ op_9118, 0, 37144 }, /* SUB */
{ op_9120, 0, 37152 }, /* SUB */
{ op_9128, 0, 37160 }, /* SUB */
{ op_9130, 0, 37168 }, /* SUB */
{ op_9138, 0, 37176 }, /* SUB */
{ op_9139, 0, 37177 }, /* SUB */
{ op_9140, 0, 37184 }, /* SUBX */
{ op_9148, 0, 37192 }, /* SUBX */
{ op_9150, 0, 37200 }, /* SUB */
{ op_9158, 0, 37208 }, /* SUB */
{ op_9160, 0, 37216 }, /* SUB */
{ op_9168, 0, 37224 }, /* SUB */
{ op_9170, 0, 37232 }, /* SUB */
{ op_9178, 0, 37240 }, /* SUB */
{ op_9179, 0, 37241 }, /* SUB */
{ op_9180, 0, 37248 }, /* SUBX */
{ op_9188, 0, 37256 }, /* SUBX */
{ op_9190, 0, 37264 }, /* SUB */
{ op_9198, 0, 37272 }, /* SUB */
{ op_91a0, 0, 37280 }, /* SUB */
{ op_91a8, 0, 37288 }, /* SUB */
{ op_91b0, 0, 37296 }, /* SUB */
{ op_91b8, 0, 37304 }, /* SUB */
{ op_91b9, 0, 37305 }, /* SUB */
{ op_91c0, 0, 37312 }, /* SUBA */
{ op_91c8, 0, 37320 }, /* SUBA */
{ op_91d0, 0, 37328 }, /* SUBA */
{ op_91d8, 0, 37336 }, /* SUBA */
{ op_91e0, 0, 37344 }, /* SUBA */
{ op_91e8, 0, 37352 }, /* SUBA */
{ op_91f0, 0, 37360 }, /* SUBA */
{ op_91f8, 0, 37368 }, /* SUBA */
{ op_91f9, 0, 37369 }, /* SUBA */
{ op_91fa, 0, 37370 }, /* SUBA */
{ op_91fb, 0, 37371 }, /* SUBA */
{ op_91fc, 0, 37372 }, /* SUBA */
{ op_b000, 0, 45056 }, /* CMP */
{ op_b010, 0, 45072 }, /* CMP */
{ op_b018, 0, 45080 }, /* CMP */
{ op_b020, 0, 45088 }, /* CMP */
{ op_b028, 0, 45096 }, /* CMP */
{ op_b030, 0, 45104 }, /* CMP */
{ op_b038, 0, 45112 }, /* CMP */
{ op_b039, 0, 45113 }, /* CMP */
{ op_b03a, 0, 45114 }, /* CMP */
{ op_b03b, 0, 45115 }, /* CMP */
{ op_b03c, 0, 45116 }, /* CMP */
{ op_b040, 0, 45120 }, /* CMP */
{ op_b048, 0, 45128 }, /* CMP */
{ op_b050, 0, 45136 }, /* CMP */
{ op_b058, 0, 45144 }, /* CMP */
{ op_b060, 0, 45152 }, /* CMP */
{ op_b068, 0, 45160 }, /* CMP */
{ op_b070, 0, 45168 }, /* CMP */
{ op_b078, 0, 45176 }, /* CMP */
{ op_b079, 0, 45177 }, /* CMP */
{ op_b07a, 0, 45178 }, /* CMP */
{ op_b07b, 0, 45179 }, /* CMP */
{ op_b07c, 0, 45180 }, /* CMP */
{ op_b080, 0, 45184 }, /* CMP */
{ op_b088, 0, 45192 }, /* CMP */
{ op_b090, 0, 45200 }, /* CMP */
{ op_b098, 0, 45208 }, /* CMP */
{ op_b0a0, 0, 45216 }, /* CMP */
{ op_b0a8, 0, 45224 }, /* CMP */
{ op_b0b0, 0, 45232 }, /* CMP */
{ op_b0b8, 0, 45240 }, /* CMP */
{ op_b0b9, 0, 45241 }, /* CMP */
{ op_b0ba, 0, 45242 }, /* CMP */
{ op_b0bb, 0, 45243 }, /* CMP */
{ op_b0bc, 0, 45244 }, /* CMP */
{ op_b0c0, 0, 45248 }, /* CMPA */
{ op_b0c8, 0, 45256 }, /* CMPA */
{ op_b0d0, 0, 45264 }, /* CMPA */
{ op_b0d8, 0, 45272 }, /* CMPA */
{ op_b0e0, 0, 45280 }, /* CMPA */
{ op_b0e8, 0, 45288 }, /* CMPA */
{ op_b0f0, 0, 45296 }, /* CMPA */
{ op_b0f8, 0, 45304 }, /* CMPA */
{ op_b0f9, 0, 45305 }, /* CMPA */
{ op_b0fa, 0, 45306 }, /* CMPA */
{ op_b0fb, 0, 45307 }, /* CMPA */
{ op_b0fc, 0, 45308 }, /* CMPA */
{ op_b100, 0, 45312 }, /* EOR */
{ op_b108, 0, 45320 }, /* CMPM */
{ op_b110, 0, 45328 }, /* EOR */
{ op_b118, 0, 45336 }, /* EOR */
{ op_b120, 0, 45344 }, /* EOR */
{ op_b128, 0, 45352 }, /* EOR */
{ op_b130, 0, 45360 }, /* EOR */
{ op_b138, 0, 45368 }, /* EOR */
{ op_b139, 0, 45369 }, /* EOR */
{ op_b140, 0, 45376 }, /* EOR */
{ op_b148, 0, 45384 }, /* CMPM */
{ op_b150, 0, 45392 }, /* EOR */
{ op_b158, 0, 45400 }, /* EOR */
{ op_b160, 0, 45408 }, /* EOR */
{ op_b168, 0, 45416 }, /* EOR */
{ op_b170, 0, 45424 }, /* EOR */
{ op_b178, 0, 45432 }, /* EOR */
{ op_b179, 0, 45433 }, /* EOR */
{ op_b180, 0, 45440 }, /* EOR */
{ op_b188, 0, 45448 }, /* CMPM */
{ op_b190, 0, 45456 }, /* EOR */
{ op_b198, 0, 45464 }, /* EOR */
{ op_b1a0, 0, 45472 }, /* EOR */
{ op_b1a8, 0, 45480 }, /* EOR */
{ op_b1b0, 0, 45488 }, /* EOR */
{ op_b1b8, 0, 45496 }, /* EOR */
{ op_b1b9, 0, 45497 }, /* EOR */
{ op_b1c0, 0, 45504 }, /* CMPA */
{ op_b1c8, 0, 45512 }, /* CMPA */
{ op_b1d0, 0, 45520 }, /* CMPA */
{ op_b1d8, 0, 45528 }, /* CMPA */
{ op_b1e0, 0, 45536 }, /* CMPA */
{ op_b1e8, 0, 45544 }, /* CMPA */
{ op_b1f0, 0, 45552 }, /* CMPA */
{ op_b1f8, 0, 45560 }, /* CMPA */
{ op_b1f9, 0, 45561 }, /* CMPA */
{ op_b1fa, 0, 45562 }, /* CMPA */
{ op_b1fb, 0, 45563 }, /* CMPA */
{ op_b1fc, 0, 45564 }, /* CMPA */
{ op_c000, 0, 49152 }, /* AND */
{ op_c010, 0, 49168 }, /* AND */
{ op_c018, 0, 49176 }, /* AND */
{ op_c020, 0, 49184 }, /* AND */
{ op_c028, 0, 49192 }, /* AND */
{ op_c030, 0, 49200 }, /* AND */
{ op_c038, 0, 49208 }, /* AND */
{ op_c039, 0, 49209 }, /* AND */
{ op_c03a, 0, 49210 }, /* AND */
{ op_c03b, 0, 49211 }, /* AND */
{ op_c03c, 0, 49212 }, /* AND */
{ op_c040, 0, 49216 }, /* AND */
{ op_c050, 0, 49232 }, /* AND */
{ op_c058, 0, 49240 }, /* AND */
{ op_c060, 0, 49248 }, /* AND */
{ op_c068, 0, 49256 }, /* AND */
{ op_c070, 0, 49264 }, /* AND */
{ op_c078, 0, 49272 }, /* AND */
{ op_c079, 0, 49273 }, /* AND */
{ op_c07a, 0, 49274 }, /* AND */
{ op_c07b, 0, 49275 }, /* AND */
{ op_c07c, 0, 49276 }, /* AND */
{ op_c080, 0, 49280 }, /* AND */
{ op_c090, 0, 49296 }, /* AND */
{ op_c098, 0, 49304 }, /* AND */
{ op_c0a0, 0, 49312 }, /* AND */
{ op_c0a8, 0, 49320 }, /* AND */
{ op_c0b0, 0, 49328 }, /* AND */
{ op_c0b8, 0, 49336 }, /* AND */
{ op_c0b9, 0, 49337 }, /* AND */
{ op_c0ba, 0, 49338 }, /* AND */
{ op_c0bb, 0, 49339 }, /* AND */
{ op_c0bc, 0, 49340 }, /* AND */
{ op_c0c0, 0, 49344 }, /* MULU */
{ op_c0d0, 0, 49360 }, /* MULU */
{ op_c0d8, 0, 49368 }, /* MULU */
{ op_c0e0, 0, 49376 }, /* MULU */
{ op_c0e8, 0, 49384 }, /* MULU */
{ op_c0f0, 0, 49392 }, /* MULU */
{ op_c0f8, 0, 49400 }, /* MULU */
{ op_c0f9, 0, 49401 }, /* MULU */
{ op_c0fa, 0, 49402 }, /* MULU */
{ op_c0fb, 0, 49403 }, /* MULU */
{ op_c0fc, 0, 49404 }, /* MULU */
{ op_c100, 0, 49408 }, /* ABCD */
{ op_c108, 0, 49416 }, /* ABCD */
{ op_c110, 0, 49424 }, /* AND */
{ op_c118, 0, 49432 }, /* AND */
{ op_c120, 0, 49440 }, /* AND */
{ op_c128, 0, 49448 }, /* AND */
{ op_c130, 0, 49456 }, /* AND */
{ op_c138, 0, 49464 }, /* AND */
{ op_c139, 0, 49465 }, /* AND */
{ op_c140, 0, 49472 }, /* EXG */
{ op_c148, 0, 49480 }, /* EXG */
{ op_c150, 0, 49488 }, /* AND */
{ op_c158, 0, 49496 }, /* AND */
{ op_c160, 0, 49504 }, /* AND */
{ op_c168, 0, 49512 }, /* AND */
{ op_c170, 0, 49520 }, /* AND */
{ op_c178, 0, 49528 }, /* AND */
{ op_c179, 0, 49529 }, /* AND */
{ op_c188, 0, 49544 }, /* EXG */
{ op_c190, 0, 49552 }, /* AND */
{ op_c198, 0, 49560 }, /* AND */
{ op_c1a0, 0, 49568 }, /* AND */
{ op_c1a8, 0, 49576 }, /* AND */
{ op_c1b0, 0, 49584 }, /* AND */
{ op_c1b8, 0, 49592 }, /* AND */
{ op_c1b9, 0, 49593 }, /* AND */
{ op_c1c0, 0, 49600 }, /* MULS */
{ op_c1d0, 0, 49616 }, /* MULS */
{ op_c1d8, 0, 49624 }, /* MULS */
{ op_c1e0, 0, 49632 }, /* MULS */
{ op_c1e8, 0, 49640 }, /* MULS */
{ op_c1f0, 0, 49648 }, /* MULS */
{ op_c1f8, 0, 49656 }, /* MULS */
{ op_c1f9, 0, 49657 }, /* MULS */
{ op_c1fa, 0, 49658 }, /* MULS */
{ op_c1fb, 0, 49659 }, /* MULS */
{ op_c1fc, 0, 49660 }, /* MULS */
{ op_d000, 0, 53248 }, /* ADD */
{ op_d010, 0, 53264 }, /* ADD */
{ op_d018, 0, 53272 }, /* ADD */
{ op_d020, 0, 53280 }, /* ADD */
{ op_d028, 0, 53288 }, /* ADD */
{ op_d030, 0, 53296 }, /* ADD */
{ op_d038, 0, 53304 }, /* ADD */
{ op_d039, 0, 53305 }, /* ADD */
{ op_d03a, 0, 53306 }, /* ADD */
{ op_d03b, 0, 53307 }, /* ADD */
{ op_d03c, 0, 53308 }, /* ADD */
{ op_d040, 0, 53312 }, /* ADD */
{ op_d048, 0, 53320 }, /* ADD */
{ op_d050, 0, 53328 }, /* ADD */
{ op_d058, 0, 53336 }, /* ADD */
{ op_d060, 0, 53344 }, /* ADD */
{ op_d068, 0, 53352 }, /* ADD */
{ op_d070, 0, 53360 }, /* ADD */
{ op_d078, 0, 53368 }, /* ADD */
{ op_d079, 0, 53369 }, /* ADD */
{ op_d07a, 0, 53370 }, /* ADD */
{ op_d07b, 0, 53371 }, /* ADD */
{ op_d07c, 0, 53372 }, /* ADD */
{ op_d080, 0, 53376 }, /* ADD */
{ op_d088, 0, 53384 }, /* ADD */
{ op_d090, 0, 53392 }, /* ADD */
{ op_d098, 0, 53400 }, /* ADD */
{ op_d0a0, 0, 53408 }, /* ADD */
{ op_d0a8, 0, 53416 }, /* ADD */
{ op_d0b0, 0, 53424 }, /* ADD */
{ op_d0b8, 0, 53432 }, /* ADD */
{ op_d0b9, 0, 53433 }, /* ADD */
{ op_d0ba, 0, 53434 }, /* ADD */
{ op_d0bb, 0, 53435 }, /* ADD */
{ op_d0bc, 0, 53436 }, /* ADD */
{ op_d0c0, 0, 53440 }, /* ADDA */
{ op_d0c8, 0, 53448 }, /* ADDA */
{ op_d0d0, 0, 53456 }, /* ADDA */
{ op_d0d8, 0, 53464 }, /* ADDA */
{ op_d0e0, 0, 53472 }, /* ADDA */
{ op_d0e8, 0, 53480 }, /* ADDA */
{ op_d0f0, 0, 53488 }, /* ADDA */
{ op_d0f8, 0, 53496 }, /* ADDA */
{ op_d0f9, 0, 53497 }, /* ADDA */
{ op_d0fa, 0, 53498 }, /* ADDA */
{ op_d0fb, 0, 53499 }, /* ADDA */
{ op_d0fc, 0, 53500 }, /* ADDA */
{ op_d100, 0, 53504 }, /* ADDX */
{ op_d108, 0, 53512 }, /* ADDX */
{ op_d110, 0, 53520 }, /* ADD */
{ op_d118, 0, 53528 }, /* ADD */
{ op_d120, 0, 53536 }, /* ADD */
{ op_d128, 0, 53544 }, /* ADD */
{ op_d130, 0, 53552 }, /* ADD */
{ op_d138, 0, 53560 }, /* ADD */
{ op_d139, 0, 53561 }, /* ADD */
{ op_d140, 0, 53568 }, /* ADDX */
{ op_d148, 0, 53576 }, /* ADDX */
{ op_d150, 0, 53584 }, /* ADD */
{ op_d158, 0, 53592 }, /* ADD */
{ op_d160, 0, 53600 }, /* ADD */
{ op_d168, 0, 53608 }, /* ADD */
{ op_d170, 0, 53616 }, /* ADD */
{ op_d178, 0, 53624 }, /* ADD */
{ op_d179, 0, 53625 }, /* ADD */
{ op_d180, 0, 53632 }, /* ADDX */
{ op_d188, 0, 53640 }, /* ADDX */
{ op_d190, 0, 53648 }, /* ADD */
{ op_d198, 0, 53656 }, /* ADD */
{ op_d1a0, 0, 53664 }, /* ADD */
{ op_d1a8, 0, 53672 }, /* ADD */
{ op_d1b0, 0, 53680 }, /* ADD */
{ op_d1b8, 0, 53688 }, /* ADD */
{ op_d1b9, 0, 53689 }, /* ADD */
{ op_d1c0, 0, 53696 }, /* ADDA */
{ op_d1c8, 0, 53704 }, /* ADDA */
{ op_d1d0, 0, 53712 }, /* ADDA */
{ op_d1d8, 0, 53720 }, /* ADDA */
{ op_d1e0, 0, 53728 }, /* ADDA */
{ op_d1e8, 0, 53736 }, /* ADDA */
{ op_d1f0, 0, 53744 }, /* ADDA */
{ op_d1f8, 0, 53752 }, /* ADDA */
{ op_d1f9, 0, 53753 }, /* ADDA */
{ op_d1fa, 0, 53754 }, /* ADDA */
{ op_d1fb, 0, 53755 }, /* ADDA */
{ op_d1fc, 0, 53756 }, /* ADDA */
{ op_e000, 0, 57344 }, /* ASR */
{ op_e008, 0, 57352 }, /* LSR */
{ op_e010, 0, 57360 }, /* ROXR */
{ op_e018, 0, 57368 }, /* ROR */
{ op_e020, 0, 57376 }, /* ASR */
{ op_e028, 0, 57384 }, /* LSR */
{ op_e030, 0, 57392 }, /* ROXR */
{ op_e038, 0, 57400 }, /* ROR */
{ op_e040, 0, 57408 }, /* ASR */
{ op_e048, 0, 57416 }, /* LSR */
{ op_e050, 0, 57424 }, /* ROXR */
{ op_e058, 0, 57432 }, /* ROR */
{ op_e060, 0, 57440 }, /* ASR */
{ op_e068, 0, 57448 }, /* LSR */
{ op_e070, 0, 57456 }, /* ROXR */
{ op_e078, 0, 57464 }, /* ROR */
{ op_e080, 0, 57472 }, /* ASR */
{ op_e088, 0, 57480 }, /* LSR */
{ op_e090, 0, 57488 }, /* ROXR */
{ op_e098, 0, 57496 }, /* ROR */
{ op_e0a0, 0, 57504 }, /* ASR */
{ op_e0a8, 0, 57512 }, /* LSR */
{ op_e0b0, 0, 57520 }, /* ROXR */
{ op_e0b8, 0, 57528 }, /* ROR */
{ op_e0d0, 0, 57552 }, /* ASRW */
{ op_e0d8, 0, 57560 }, /* ASRW */
{ op_e0e0, 0, 57568 }, /* ASRW */
{ op_e0e8, 0, 57576 }, /* ASRW */
{ op_e0f0, 0, 57584 }, /* ASRW */
{ op_e0f8, 0, 57592 }, /* ASRW */
{ op_e0f9, 0, 57593 }, /* ASRW */
{ op_e100, 0, 57600 }, /* ASL */
{ op_e108, 0, 57608 }, /* LSL */
{ op_e110, 0, 57616 }, /* ROXL */
{ op_e118, 0, 57624 }, /* ROL */
{ op_e120, 0, 57632 }, /* ASL */
{ op_e128, 0, 57640 }, /* LSL */
{ op_e130, 0, 57648 }, /* ROXL */
{ op_e138, 0, 57656 }, /* ROL */
{ op_e140, 0, 57664 }, /* ASL */
{ op_e148, 0, 57672 }, /* LSL */
{ op_e150, 0, 57680 }, /* ROXL */
{ op_e158, 0, 57688 }, /* ROL */
{ op_e160, 0, 57696 }, /* ASL */
{ op_e168, 0, 57704 }, /* LSL */
{ op_e170, 0, 57712 }, /* ROXL */
{ op_e178, 0, 57720 }, /* ROL */
{ op_e180, 0, 57728 }, /* ASL */
{ op_e188, 0, 57736 }, /* LSL */
{ op_e190, 0, 57744 }, /* ROXL */
{ op_e198, 0, 57752 }, /* ROL */
{ op_e1a0, 0, 57760 }, /* ASL */
{ op_e1a8, 0, 57768 }, /* LSL */
{ op_e1b0, 0, 57776 }, /* ROXL */
{ op_e1b8, 0, 57784 }, /* ROL */
{ op_e1d0, 0, 57808 }, /* ASLW */
{ op_e1d8, 0, 57816 }, /* ASLW */
{ op_e1e0, 0, 57824 }, /* ASLW */
{ op_e1e8, 0, 57832 }, /* ASLW */
{ op_e1f0, 0, 57840 }, /* ASLW */
{ op_e1f8, 0, 57848 }, /* ASLW */
{ op_e1f9, 0, 57849 }, /* ASLW */
{ op_e2d0, 0, 58064 }, /* LSRW */
{ op_e2d8, 0, 58072 }, /* LSRW */
{ op_e2e0, 0, 58080 }, /* LSRW */
{ op_e2e8, 0, 58088 }, /* LSRW */
{ op_e2f0, 0, 58096 }, /* LSRW */
{ op_e2f8, 0, 58104 }, /* LSRW */
{ op_e2f9, 0, 58105 }, /* LSRW */
{ op_e3d0, 0, 58320 }, /* LSLW */
{ op_e3d8, 0, 58328 }, /* LSLW */
{ op_e3e0, 0, 58336 }, /* LSLW */
{ op_e3e8, 0, 58344 }, /* LSLW */
{ op_e3f0, 0, 58352 }, /* LSLW */
{ op_e3f8, 0, 58360 }, /* LSLW */
{ op_e3f9, 0, 58361 }, /* LSLW */
{ op_e4d0, 0, 58576 }, /* ROXRW */
{ op_e4d8, 0, 58584 }, /* ROXRW */
{ op_e4e0, 0, 58592 }, /* ROXRW */
{ op_e4e8, 0, 58600 }, /* ROXRW */
{ op_e4f0, 0, 58608 }, /* ROXRW */
{ op_e4f8, 0, 58616 }, /* ROXRW */
{ op_e4f9, 0, 58617 }, /* ROXRW */
{ op_e5d0, 0, 58832 }, /* ROXLW */
{ op_e5d8, 0, 58840 }, /* ROXLW */
{ op_e5e0, 0, 58848 }, /* ROXLW */
{ op_e5e8, 0, 58856 }, /* ROXLW */
{ op_e5f0, 0, 58864 }, /* ROXLW */
{ op_e5f8, 0, 58872 }, /* ROXLW */
{ op_e5f9, 0, 58873 }, /* ROXLW */
{ op_e6d0, 0, 59088 }, /* RORW */
{ op_e6d8, 0, 59096 }, /* RORW */
{ op_e6e0, 0, 59104 }, /* RORW */
{ op_e6e8, 0, 59112 }, /* RORW */
{ op_e6f0, 0, 59120 }, /* RORW */
{ op_e6f8, 0, 59128 }, /* RORW */
{ op_e6f9, 0, 59129 }, /* RORW */
{ op_e7d0, 0, 59344 }, /* ROLW */
{ op_e7d8, 0, 59352 }, /* ROLW */
{ op_e7e0, 0, 59360 }, /* ROLW */
{ op_e7e8, 0, 59368 }, /* ROLW */
{ op_e7f0, 0, 59376 }, /* ROLW */
{ op_e7f8, 0, 59384 }, /* ROLW */
{ op_e7f9, 0, 59385 }, /* ROLW */
{ 0, 0, 0 }};
cpuop_func *cpufunctbl[65536];

struct mnemolookup lookuptab[] = {
    { i_ILLG, "ILLEGAL" },
    { i_OR, "OR" },
    { i_CHK, "CHK" },
    { i_CHK2, "CHK2" },
    { i_AND, "AND" },
    { i_EOR, "EOR" },
    { i_ORSR, "ORSR" },
    { i_ANDSR, "ANDSR" },
    { i_EORSR, "EORSR" },
    { i_SUB, "SUB" },
    { i_SUBA, "SUBA" },
    { i_SUBX, "SUBX" },
    { i_SBCD, "SBCD" },
    { i_ADD, "ADD" },
    { i_ADDA, "ADDA" },
    { i_ADDX, "ADDX" },
    { i_ABCD, "ABCD" },
    { i_NEG, "NEG" },
    { i_NEGX, "NEGX" },
    { i_NBCD, "NBCD" },
    { i_CLR, "CLR" },
    { i_NOT, "NOT" },
    { i_TST, "TST" },
    { i_BTST, "BTST" },
    { i_BCHG, "BCHG" },
    { i_BCLR, "BCLR" },
    { i_BSET, "BSET" },
    { i_CMP, "CMP" },
    { i_CMPM, "CMPM" },
    { i_CMPA, "CMPA" },
    { i_MVPRM, "MVPRM" },
    { i_MVPMR, "MVPMR" },
    { i_MOVE, "MOVE" },
    { i_MOVEA, "MOVEA" },
    { i_MVSR2, "MVSR2" },
    { i_MV2SR, "MV2SR" },
    { i_SWAP, "SWAP" },
    { i_EXG, "EXG" },
    { i_EXT, "EXT" },
    { i_MVMEL, "MVMEL" },
    { i_MVMLE, "MVMLE" },
    { i_TRAP, "TRAP" },
    { i_MVR2USP, "MVR2USP" },
    { i_MVUSP2R, "MVUSP2R" },
    { i_NOP, "NOP" },
    { i_RESET, "RESET" },
    { i_RTE, "RTE" },
    { i_RTD, "RTD" },
    { i_LINK, "LINK" },
    { i_UNLK, "UNLK" },
    { i_RTS, "RTS" },
    { i_STOP, "STOP" },
    { i_TRAPV, "TRAPV" },
    { i_RTR, "RTR" },
    { i_JSR, "JSR" },
    { i_JMP, "JMP" },
    { i_BSR, "BSR" },
    { i_Bcc, "Bcc" },
    { i_LEA, "LEA" },
    { i_PEA, "PEA" },
    { i_DBcc, "DBcc" },
    { i_Scc, "Scc" },
    { i_DIVU, "DIVU" },
    { i_DIVS, "DIVS" },
    { i_MULU, "MULU" },
    { i_MULS, "MULS" },
    { i_ASR, "ASR" },
    { i_ASL, "ASL" },
    { i_LSR, "LSR" },
    { i_LSL, "LSL" },
    { i_ROL, "ROL" },
    { i_ROR, "ROR" },
    { i_ROXL, "ROXL" },
    { i_ROXR, "ROXR" },
    { i_ASRW, "ASRW" },
    { i_ASLW, "ASLW" },
    { i_LSRW, "LSRW" },
    { i_LSLW, "LSLW" },
    { i_ROLW, "ROLW" },
    { i_RORW, "RORW" },
    { i_ROXLW, "ROXLW" },
    { i_ROXRW, "ROXRW" },

    { i_MOVE2C, "MOVE2C" },
    { i_MOVEC2, "MOVEC2" },
    { i_CAS, "CAS" },
    { i_CAS2, "CAS2" },
    { i_MULL, "MULL" },
    { i_DIVL, "DIVL" },
    { i_BFTST, "BFTST" },
    { i_BFEXTU, "BFEXTU" },
    { i_BFCHG, "BFCHG" },
    { i_BFEXTS, "BFEXTS" },
    { i_BFCLR, "BFCLR" },
    { i_BFFFO, "BFFFO" },
    { i_BFSET, "BFSET" },
    { i_BFINS, "BFINS" },
    { i_PACK, "PACK" },
    { i_UNPK, "UNPK" },
    { i_TAS, "TAS" },
    { i_BKPT, "BKPT" },
    { i_CALLM, "CALLM" },
    { i_RTM, "RTM" },
    { i_TRAPcc, "TRAPcc" },
    { i_MOVES, "MOVES" },
    { i_FPP, "FPP" },
    { i_FDBcc, "FDBcc" },
    { i_FScc, "FScc" },
    { i_FTRAPcc, "FTRAPcc" },
    { i_FBcc, "FBcc" },
    { i_FBcc, "FBcc" },
    { i_FSAVE, "FSAVE" },
    { i_FRESTORE, "FRESTORE" },
    { i_MMUOP, "MMUOP" },
    { i_ILLG, "" },
};

static instr __table68k_static[65536];

struct instr *table68k=(struct instr *)&__table68k_static[0];

INLINE amodes mode_from_str(const char *str)
{
    if (strncmp(str,"Dreg",4) == 0) return Dreg;
    if (strncmp(str,"Areg",4) == 0) return Areg;
    if (strncmp(str,"Aind",4) == 0) return Aind;
    if (strncmp(str,"Apdi",4) == 0) return Apdi;
    if (strncmp(str,"Aipi",4) == 0) return Aipi;
    if (strncmp(str,"Ad16",4) == 0) return Ad16;
    if (strncmp(str,"Ad8r",4) == 0) return Ad8r;
    if (strncmp(str,"absw",4) == 0) return absw;
    if (strncmp(str,"absl",4) == 0) return absl;
    if (strncmp(str,"PC16",4) == 0) return PC16;
    if (strncmp(str,"PC8r",4) == 0) return PC8r;
    if (strncmp(str,"Immd",4) == 0) return imm;
    abort();
    return(Dreg);
}

INLINE amodes mode_from_mr(int mode, int reg)
{
    switch(mode) {
     case 0: return Dreg;
     case 1: return Areg;
     case 2: return Aind;
     case 3: return Aipi;
     case 4: return Apdi;
     case 5: return Ad16;
     case 6: return Ad8r;
     case 7:
        switch(reg) {
         case 0: return absw;
         case 1: return absl;
         case 2: return PC16;
         case 3: return PC8r;
         case 4: return imm;
         case 5:
         case 6:
         case 7: return am_illg;
        }
    }
    abort();
    return(Dreg);
}

static void build_insn(int insn)
{
    int find = -1;
    int variants;
    struct instr_def id;
    const char *opcstr;

    id = defs68k[insn];
    opcstr = id.opcstr;
    for (variants = 0; variants < (1 << id.n_variable); variants++) {
        int bitcnt[lastbit];
        int bitval[lastbit];
        int bitpos[lastbit];
        int i;
        UWORD opc = id.bits;
        UWORD msk, vmsk;
        int pos = 0;
        int mnp = 0;
        int bitno = 0;
        char mnemonic[10];

        wordsizes sz = sz_unknown;
        int srcgather = 0, dstgather = 0;
        int usesrc = 0, usedst = 0;
        int srctype = 0;
        int srcpos = -1, dstpos = -1;

        amodes srcmode = am_unknown, destmode = am_unknown;
        int srcreg = -1, destreg = -1;

        for(i = 0; i < lastbit; i++)
            bitcnt[i] = bitval[i] = 0;

        vmsk = 1 << id.n_variable;

        for(i = 0, msk = 0x8000; i < 16; i++, msk >>= 1) {
            if (!(msk & id.mask)) {
                int currbit = id.bitpos[bitno++];
                int bit_set;
                vmsk >>= 1;
                bit_set = variants & vmsk ? 1 : 0;
                if (bit_set)
                    opc |= msk;
                bitpos[currbit] = 15 - i;
                bitcnt[currbit]++;
                bitval[currbit] <<= 1;
                bitval[currbit] |= bit_set;
            }
        }

        if (bitval[bitj] == 0) bitval[bitj] = 8;
        /* first check whether this one does not match after all */
        if (bitval[bitz] == 3 || bitval[bitC] == 1)
            continue;
        if (bitcnt[bitI] && (bitval[bitI] == 0x00 || bitval[bitI] == 0xff))
            continue;

        /* bitI and bitC get copied to biti and bitc */
        if (bitcnt[bitI]) {
            bitval[biti] = bitval[bitI]; bitpos[biti] = bitpos[bitI];
        }
        if (bitcnt[bitC])
            bitval[bitc] = bitval[bitC];

        pos = 0;
        while (opcstr[pos] && !isspace(opcstr[pos])) {
            if (opcstr[pos] == '.') {
                pos++;
                switch(opcstr[pos]) {

                 case 'B': sz = sz_byte; break;
                 case 'W': sz = sz_word; break;
                 case 'L': sz = sz_long; break;
                 case 'z':
                    switch(bitval[bitz]) {
                     case 0: sz = sz_byte; break;
                     case 1: sz = sz_word; break;
                     case 2: sz = sz_long; break;
                     default: abort();
                    }
                    break;
                 default: abort();
                }
            } else {
                mnemonic[mnp] = opcstr[pos];
                if(mnemonic[mnp] == 'f') {
                    find = -1;
                    switch(bitval[bitf]) {
                     case 0: mnemonic[mnp] = 'R'; break;
                     case 1: mnemonic[mnp] = 'L'; break;
                     default: abort();
                    }
                }
                mnp++;
            }
            pos++;
        }
        mnemonic[mnp] = 0;

        /* now, we have read the mnemonic and the size */
        while (opcstr[pos] && isspace(opcstr[pos]))
            pos++;

        /* A goto a day keeps the D******a away. */
        if (opcstr[pos] == 0)
            goto endofline;

        /* parse the source address */
        usesrc = 1;
        switch(opcstr[pos++]) {
         case 'D':
            srcmode = Dreg;
            switch (opcstr[pos++]) {
             case 'r': srcreg = bitval[bitr]; srcgather = 1; srcpos = bitpos[bitr]; break;
             case 'R': srcreg = bitval[bitR]; srcgather = 1; srcpos = bitpos[bitR]; break;
             default: abort();
            }

            break;
         case 'A':
            srcmode = Areg;
            switch (opcstr[pos++]) {
             case 'r': srcreg = bitval[bitr]; srcgather = 1; srcpos = bitpos[bitr]; break;
             case 'R': srcreg = bitval[bitR]; srcgather = 1; srcpos = bitpos[bitR]; break;
             default: abort();
            }
            switch (opcstr[pos]) {
             case 'p': srcmode = Apdi; pos++; break;
             case 'P': srcmode = Aipi; pos++; break;
            }
            break;
         case '#':
            switch(opcstr[pos++]) {
             case 'z': srcmode = imm; break;
             case '0': srcmode = imm0; break;
             case '1': srcmode = imm1; break;
             case '2': srcmode = imm2; break;
             case 'i': srcmode = immi; srcreg = (LONG)(BYTE)bitval[biti];
                if (CPU_EMU_SIZE < 4) {
                    /* Used for branch instructions */
                    srctype = 1;
                    srcgather = 1;
                    srcpos = bitpos[biti];
                }
                break;
             case 'j': srcmode = immi; srcreg = bitval[bitj];
                if (CPU_EMU_SIZE < 3) {
                    /* 1..8 for ADDQ/SUBQ and rotshi insns */
                    srcgather = 1;
                    srctype = 3;
                    srcpos = bitpos[bitj];
                }
                break;
             case 'J': srcmode = immi; srcreg = bitval[bitJ];
                if (CPU_EMU_SIZE < 5) {
                    /* 0..15 */
                    srcgather = 1;
                    srctype = 2;
                    srcpos = bitpos[bitJ];
                }
                break;
             case 'k': srcmode = immi; srcreg = bitval[bitk];
                if (CPU_EMU_SIZE < 3) {
                    srcgather = 1;
                    srctype = 4;
                    srcpos = bitpos[bitk];
                }
                break;
             case 'K': srcmode = immi; srcreg = bitval[bitK];
                if (CPU_EMU_SIZE < 5) {
                    /* 0..15 */
                    srcgather = 1;
                    srctype = 5;
                    srcpos = bitpos[bitK];
                }
                break;
             default: abort();
            }
            break;
         case 'd':
            srcreg = bitval[bitD];
            srcmode = mode_from_mr(bitval[bitd],bitval[bitD]);
            if (srcmode == am_illg) continue;
            if (CPU_EMU_SIZE < 2 &&
                (srcmode == Areg || srcmode == Dreg || srcmode == Aind
                 || srcmode == Ad16 || srcmode == Ad8r || srcmode == Aipi
                 || srcmode == Apdi))
            {
                srcgather = 1; srcpos = bitpos[bitD];
            }
            if (opcstr[pos] == '[') {
                pos++;
                if (opcstr[pos] == '!') {
                    /* exclusion */
                    do {
                        pos++;
                        if (mode_from_str(opcstr+pos) == srcmode)
                            goto nomatch;
                        pos += 4;
                    } while (opcstr[pos] == ',');
                    pos++;
                } else {
                    if (opcstr[pos+4] == '-') {
                        /* replacement */
                        if (mode_from_str(opcstr+pos) == srcmode)
                            srcmode = mode_from_str(opcstr+pos+5);
                        else
                            goto nomatch;
                        pos += 10;
                    } else {
                        /* normal */
                        while(mode_from_str(opcstr+pos) != srcmode) {
                            pos += 4;
                            if (opcstr[pos] == ']')
                                goto nomatch;
                            pos++;
                        }
                        while(opcstr[pos] != ']') pos++;
                        pos++;
                        break;
                    }
                }
            }
            /* Some addressing modes are invalid as destination */
            if (srcmode == imm || srcmode == PC16 || srcmode == PC8r)
                goto nomatch;
            break;
         case 's':
            srcreg = bitval[bitS];
            srcmode = mode_from_mr(bitval[bits],bitval[bitS]);

            if (srcmode == am_illg) continue;
            if (CPU_EMU_SIZE < 2 &&
                (srcmode == Areg || srcmode == Dreg || srcmode == Aind
                 || srcmode == Ad16 || srcmode == Ad8r || srcmode == Aipi
                 || srcmode == Apdi))
            {
                srcgather = 1; srcpos = bitpos[bitS];
            }
            if (opcstr[pos] == '[') {
                pos++;
                if (opcstr[pos] == '!') {
                    /* exclusion */
                    do {
                        pos++;
                        if (mode_from_str(opcstr+pos) == srcmode)
                            goto nomatch;
                        pos += 4;
                    } while (opcstr[pos] == ',');
                    pos++;
                } else {
                    if (opcstr[pos+4] == '-') {
                        /* replacement */
                        if (mode_from_str(opcstr+pos) == srcmode)
                            srcmode = mode_from_str(opcstr+pos+5);
                        else
                            goto nomatch;
                        pos += 10;
                    } else {
                        /* normal */
                        while(mode_from_str(opcstr+pos) != srcmode) {
                            pos += 4;
                            if (opcstr[pos] == ']')
                                goto nomatch;
                            pos++;
                        }
                        while(opcstr[pos] != ']') pos++;
                        pos++;
                    }
                }
            }
            break;
         default: abort();
        }
        /* safety check - might have changed */
        if (srcmode != Areg && srcmode != Dreg && srcmode != Aind
            && srcmode != Ad16 && srcmode != Ad8r && srcmode != Aipi
            && srcmode != Apdi && srcmode != immi)
        {
            srcgather = 0;
        }
        if (srcmode == Areg && sz == sz_byte)
            goto nomatch;

        if (opcstr[pos] != ',')
            goto endofline;
        pos++;

        /* parse the destination address */
        usedst = 1;
        switch(opcstr[pos++]) {
         case 'D':
            destmode = Dreg;
            switch (opcstr[pos++]) {
             case 'r': destreg = bitval[bitr]; dstgather = 1; dstpos = bitpos[bitr]; break;
             case 'R': destreg = bitval[bitR]; dstgather = 1; dstpos = bitpos[bitR]; break;
             default: abort();
            }
            break;
         case 'A':
            destmode = Areg;
            switch (opcstr[pos++]) {
             case 'r': destreg = bitval[bitr]; dstgather = 1; dstpos = bitpos[bitr]; break;
             case 'R': destreg = bitval[bitR]; dstgather = 1; dstpos = bitpos[bitR]; break;
             default: abort();
            }
            switch (opcstr[pos]) {
             case 'p': destmode = Apdi; pos++; break;
             case 'P': destmode = Aipi; pos++; break;
            }
            break;
         case '#':
            switch(opcstr[pos++]) {
             case 'z': destmode = imm; break;
             case '0': destmode = imm0; break;
             case '1': destmode = imm1; break;
             case '2': destmode = imm2; break;
             case 'i': destmode = immi; destreg = (LONG)(BYTE)bitval[biti]; break;
             case 'j': destmode = immi; destreg = bitval[bitj]; break;
             case 'J': destmode = immi; destreg = bitval[bitJ]; break;
             case 'k': destmode = immi; destreg = bitval[bitk]; break;
             case 'K': destmode = immi; destreg = bitval[bitK]; break;
             default: abort();
            }
            break;
         case 'd':
            destreg = bitval[bitD];
            destmode = mode_from_mr(bitval[bitd],bitval[bitD]);
            if(destmode == am_illg) continue;
            if (CPU_EMU_SIZE < 1 &&
                (destmode == Areg || destmode == Dreg || destmode == Aind
                 || destmode == Ad16 || destmode == Ad8r || destmode == Aipi
                 || destmode == Apdi))
            {
                dstgather = 1; dstpos = bitpos[bitD];
            }

            if (opcstr[pos] == '[') {
                pos++;
                if (opcstr[pos] == '!') {
                    /* exclusion */
                    do {
                        pos++;
                        if (mode_from_str(opcstr+pos) == destmode)
                            goto nomatch;
                        pos += 4;
                    } while (opcstr[pos] == ',');
                    pos++;
                } else {
                    if (opcstr[pos+4] == '-') {
                        /* replacement */
                        if (mode_from_str(opcstr+pos) == destmode)
                            destmode = mode_from_str(opcstr+pos+5);
                        else
                            goto nomatch;
                        pos += 10;
                    } else {
                        /* normal */
                        while(mode_from_str(opcstr+pos) != destmode) {
                            pos += 4;
                            if (opcstr[pos] == ']')
                                goto nomatch;
                            pos++;
                        }
                        while(opcstr[pos] != ']') pos++;
                        pos++;
                        break;
                    }
                }
            }
            /* Some addressing modes are invalid as destination */
            if (destmode == imm || destmode == PC16 || destmode == PC8r)
                goto nomatch;
            break;
         case 's':
            destreg = bitval[bitS];
            destmode = mode_from_mr(bitval[bits],bitval[bitS]);

            if (destmode == am_illg) continue;
            if (CPU_EMU_SIZE < 1 &&
                (destmode == Areg || destmode == Dreg || destmode == Aind
                 || destmode == Ad16 || destmode == Ad8r || destmode == Aipi
                 || destmode == Apdi))
            {
                dstgather = 1; dstpos = bitpos[bitS];
            }

            if (opcstr[pos] == '[') {
                pos++;
                if (opcstr[pos] == '!') {
                    /* exclusion */
                    do {
                        pos++;
                        if (mode_from_str(opcstr+pos) == destmode)
                            goto nomatch;
                        pos += 4;
                    } while (opcstr[pos] == ',');
                    pos++;
                } else {
                    if (opcstr[pos+4] == '-') {
                        /* replacement */
                        if (mode_from_str(opcstr+pos) == destmode)
                            destmode = mode_from_str(opcstr+pos+5);
                        else
                            goto nomatch;
                        pos += 10;
                    } else {
                        /* normal */
                        while(mode_from_str(opcstr+pos) != destmode) {
                            pos += 4;
                            if (opcstr[pos] == ']')
                                goto nomatch;
                            pos++;
                        }
                        while(opcstr[pos] != ']') pos++;
                        pos++;
                    }
                }
            }
            break;
         default: abort();
        }
        /* safety check - might have changed */
        if (destmode != Areg && destmode != Dreg && destmode != Aind
            && destmode != Ad16 && destmode != Ad8r && destmode != Aipi
            && destmode != Apdi)
        {
            dstgather = 0;
        }

        if (destmode == Areg && sz == sz_byte)
            goto nomatch;
#if 0
        if (sz == sz_byte && (destmode == Aipi || destmode == Apdi)) {
            dstgather = 0;
        }
#endif
        endofline:
        /* now, we have a match */
        /*if (table68k[opc].mnemo != i_ILLG)*/
            /*fprintf(stderr, "Double match: %x: %s\n", opc, opcstr);*/
        if (find == -1) {
            for(find = 0;; find++) {
                if (strcmp(mnemonic, lookuptab[find].name) == 0) {
                    table68k[opc].mnemo = lookuptab[find].mnemo;
                    break;
                }
                if (strlen(lookuptab[find].name) == 0) abort();
            }
        }
        else {
            table68k[opc].mnemo = lookuptab[find].mnemo;
        }
        table68k[opc].cc = bitval[bitc];
        if (table68k[opc].mnemo == i_BTST
            || table68k[opc].mnemo == i_BSET
            || table68k[opc].mnemo == i_BCLR
            || table68k[opc].mnemo == i_BCHG)
        {
            sz = destmode == Dreg ? sz_long : sz_byte;
        }
        table68k[opc].size = sz;
        table68k[opc].sreg = srcreg;
        table68k[opc].dreg = destreg;
        table68k[opc].smode = srcmode;
        table68k[opc].dmode = destmode;
        table68k[opc].spos = srcgather ? srcpos : -1;
        table68k[opc].dpos = dstgather ? dstpos : -1;
        table68k[opc].suse = usesrc;
        table68k[opc].duse = usedst;
        table68k[opc].stype = srctype;
        table68k[opc].plev = id.plevel;
        nomatch:
        /* FOO! */;
    }
}

void read_table68k ()
{
    int i;

    for(i = 0; i < 65536; i++) {
        table68k[i].mnemo = i_ILLG;
        table68k[i].handler = -1;
    }
    for (i = 0; i < n_defs68k; i++) {
        build_insn (i);
    }
}

static int mismatch;

static void handle_merges(long int opcde)
{
    UWORD smsk;
    UWORD dmsk;
    int sbitdst, dstend;
    int srcreg, dstreg;

    if (table68k[opcde].spos == -1) {
        sbitdst = 1; smsk = 0;
    }
    else {
        switch (table68k[opcde].stype) {
         case 0:
            smsk = 7; sbitdst = 8; break;
         case 1:
            smsk = 255; sbitdst = 256; break;
         case 2:
            smsk = 15; sbitdst = 16; break;
         case 3:
            smsk = 7; sbitdst = 8; break;
         case 4:
            smsk = 7; sbitdst = 8; break;
         case 5:
            smsk = 63; sbitdst = 64; break;
         default:
            smsk = 0; sbitdst = 0;
            abort();
            break;
        }
        smsk <<= table68k[opcde].spos;
    }
    if (table68k[opcde].dpos == -1) {
        dstend = 1; dmsk = 0;
    }
    else {
        dmsk = 7 << table68k[opcde].dpos;
        dstend = 8;
    }
    for (srcreg=0; srcreg < sbitdst; srcreg++) {
        for (dstreg=0; dstreg < dstend; dstreg++) {
            UWORD code = opcde;

            code = (code & ~smsk) | (srcreg << table68k[opcde].spos);
            code = (code & ~dmsk) | (dstreg << table68k[opcde].dpos);

            /* Check whether this is in fact the same instruction.
             * The instructions should never differ, except for the
             * Bcc.(BW) case. */
            if (table68k[code].mnemo != table68k[opcde].mnemo
                || table68k[code].size != table68k[opcde].size
                || table68k[code].suse != table68k[opcde].suse
                || table68k[code].duse != table68k[opcde].duse)
            {
                mismatch++; continue;
            }
            if (table68k[opcde].suse
                && (table68k[opcde].spos != table68k[code].spos
                    || table68k[opcde].smode != table68k[code].smode
                    || table68k[opcde].stype != table68k[code].stype))
            {
                mismatch++; continue;
            }
            if (table68k[opcde].duse
                && (table68k[opcde].dpos != table68k[code].dpos
                    || table68k[opcde].dmode != table68k[code].dmode))
            {
                mismatch++; continue;
            }

            if (code != opcde)
                table68k[code].handler = opcde;
        }
    }
}

void do_merges ()
{
    long int opcde;
    mismatch = 0;

    for (opcde = 0; opcde < 65536; opcde++) {
        if (table68k[opcde].handler != -1 || table68k[opcde].mnemo == i_ILLG)
            continue;
        handle_merges (opcde);
    }

}

int get_no_mismatches ()
{
    return mismatch;
}

void op_illg(ULONG opcde)
{
 /*   printf("illegal opcde:%lx at %x\n",opcde,musashik_regs.pc); */
    musashik_regs.pc-=2;
    if((opcde&0xf000)==0xf000) {
        Exception(0xb,0);
        return;
    }
    if((opcde&0xf000)==0xa000) {
        Exception(0xa,0);
        return;
    }
    /* Illegal opcde */
    Exception(4,0);
}

void BuildCPU(void)
{
    int opcde,i;

    read_table68k();
    do_merges();
    for (opcde = 0; opcde < 65536; opcde++)
        cpufunctbl[opcde] =(cpuop_func *) op_illg;
    for (i = 0; smallcputbl[i].handler != NULL; i++) {
        if (!smallcputbl[i].specific)
            cpufunctbl[smallcputbl[i].opcode] = smallcputbl[i].handler;
    }
    for (opcde = 0; opcde < 65536; opcde++) {
        cpuop_func *f;

        if (table68k[opcde].mnemo == i_ILLG)
            continue;

        if (table68k[opcde].handler != -1) {
            f = cpufunctbl[table68k[opcde].handler];
            if (f == (cpuop_func *)op_illg)
                abort();
            cpufunctbl[opcde] = f;
        }
    }
    for (i = 0; smallcputbl[i].handler != NULL; i++) {
        if (smallcputbl[i].specific)
            cpufunctbl[smallcputbl[i].opcode] = smallcputbl[i].handler;
    }
}

#endif
