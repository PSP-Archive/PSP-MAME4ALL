/****************************************************************************
 *					Texas Instruments TMS320C10 DSP Emulator				*
 *																			*
 *						Copyright (C) 1999 by Quench						*
 *		You are not allowed to distribute this software commercially.		*
 *						Written for the MAME project.						*
 *																			*
 *		NOTES : The term 'DMA' within this document, is in reference		*
 *					to Direct Memory Addressing, and NOT the usual term		*
 *					of Direct Memory Access.								*
 *				This is a word based microcontroller.						*
 *																			*
 **************************************************************************/


#include "stdlib.h"
#include "tms32010.h"
#include "cpuintrf.h"
#include "driver.h"


#define M_RDROM(A)		TMS320C10_ROM_RDMEM(A)
#define M_WRTROM(A,V)	TMS320C10_ROM_WRMEM(A,V)
#define M_RDRAM(A)		TMS320C10_RAM_RDMEM(A)
#define M_WRTRAM(A,V)	TMS320C10_RAM_WRMEM(A,V)
#define M_RDOP(A)		TMS320C10_RDOP(A)
#define M_RDOP_ARG(A)	TMS320C10_RDOP_ARG(A)
#define M_IN(A)			TMS320C10_In(A)
#define M_OUT(A,V)		TMS320C10_Out(A,V)

#define ADDR_MASK		TMS320C10_ADDR_MASK

/********  The following is the Status (Flag) register definition.  *********/
/* 15 | 14  |  13  | 12 | 11 | 10 | 9 |  8  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0  */
/* OV | OVM | INTM |  1 |  1 |  1 | 1 | ARP | 1 | 1 | 1 | 1 | 1 | 1 | 1 | DP */
#define OV_FLAG		0x8000	/* OV	(Overflow flag) 1 indicates an overflow */
#define OVM_FLAG	0x4000	/* OVM	(Overflow Mode bit) 1 forces ACC overflow to greatest positive or negative saturation value */
#define INTM_FLAG	0x2000	/* INTM	(Interrupt Mask flag) 0 enables maskable interrupts */
#define ARP_REG		0x0100	/* ARP	(Auxiliary Register Pointer) */
#define DP_REG		0x0001	/* DP	(Data memory Pointer (bank) bit) */

static UINT16	opcode=0;
static UINT8	opcode_major=0, opcode_minor, opcode_minr;	/* opcode split into MSB and LSB */
static			TMS320C10_Regs R;
	   INT32	TMS320C10_ICount;
static INT32	tmpacc;
typedef void	(*opcode_fn) (void);


#define OV		 ( R.STR & OV_FLAG)			/* OV	(Overflow flag) */
#define OVM		 ( R.STR & OVM_FLAG)		/* OVM	(Overflow Mode bit) 1 indicates an overflow */
#define INTM	 ( R.STR & INTM_FLAG)		/* INTM	(Interrupt enable flag) 0 enables maskable interrupts */
#define ARP		 ((R.STR & ARP_REG) >> 8 )	/* ARP	(Auxiliary Register Pointer) */
#define DP		 ((R.STR & DP_REG) << 7)	/* DP	(Data memory Pointer bit) */

#define dma		 (DP | (opcode_minor & 0x07f))	/* address used in direct memory access operations */
#define dmapage1 (0x80 | opcode_minor)			/* address used in direct memory access operations for sst instruction */

#define ind		 (R.AR[ARP] & 0x00ff)			/* address used in indirect memory access operations */
UINT16			 memaccess;
#define memacc	 (memaccess = (opcode_minor & 0x80) ? ind : dma)



INLINE void CLR(UINT16 flag) { R.STR &= ~flag; R.STR |= 0x1efe; }
INLINE void SET(UINT16 flag) { R.STR |=  flag; R.STR |= 0x1efe; }

INLINE void getdata(UINT8 shift,UINT8 signext)
{
	if (opcode_minor & 0x80) memaccess = ind;
	else memaccess = dma;
	R.ALU = M_RDRAM(memaccess);
	if ((signext) && (R.ALU & 0x8000)) R.ALU |= 0xffff0000;
	else R.ALU &= 0x0000ffff;
	R.ALU <<= shift;
	if (opcode_minor & 0x80) {
		if ((opcode_minor & 0x20) || (opcode_minor & 0x10)) {
			UINT16 tmpAR = R.AR[ARP];
			if (opcode_minor & 0x20) tmpAR++ ;
			if (opcode_minor & 0x10) tmpAR-- ;
			R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
		}
		if (~opcode_minor & 0x08) {
			if (opcode_minor & 1) SET(ARP_REG);
			else CLR(ARP_REG);
		}
	}
}
INLINE void getdata_lar(void)
{
	if (opcode_minor & 0x80) memaccess = ind;
	else memaccess = dma;
	R.ALU = M_RDRAM(memaccess);
	if (opcode_minor & 0x80) {
		if ((opcode_minor & 0x20) || (opcode_minor & 0x10)) {
			if ((opcode_major & 1) != ARP) {
				UINT16 tmpAR = R.AR[ARP];
				if (opcode_minor & 0x20) tmpAR++ ;
				if (opcode_minor & 0x10) tmpAR-- ;
				R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
			}
		}
		if (~opcode_minor & 0x08) {
			if (opcode_minor & 1) SET(ARP_REG);
			else CLR(ARP_REG);
		}
	}
}

INLINE void putdata(UINT16 data)
{
	if (opcode_minor & 0x80) memaccess = ind;
	else memaccess = dma;
	if (opcode_minor & 0x80) {
		if ((opcode_minor & 0x20) || (opcode_minor & 0x10)) {
			UINT16 tmpAR = R.AR[ARP];
			if (opcode_minor & 0x20) tmpAR++ ;
			if (opcode_minor & 0x10) tmpAR-- ;
			R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
		}
		if (~opcode_minor & 0x08) {
			if (opcode_minor & 1) SET(ARP_REG);
			else CLR(ARP_REG);
		}
	}
	if ((opcode_major == 0x30) || (opcode_major == 0x31)) {
		M_WRTRAM(memaccess,(R.AR[data])); }
	else M_WRTRAM(memaccess,(data&0xffff));
}
INLINE void putdata_sst(UINT16 data)
{
	if (opcode_minor & 0x80) memaccess = ind;
	else memaccess = dmapage1;
	if (opcode_minor & 0x80) {
		if ((opcode_minor & 0x20) || (opcode_minor & 0x10)) {
			UINT16 tmpAR = R.AR[ARP];
			if (opcode_minor & 0x20) tmpAR++ ;
			if (opcode_minor & 0x10) tmpAR-- ;
			R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
		}
	}
	M_WRTRAM(memaccess,(data&0xffff));
}


void M_ILLEGAL(void)
{
}


/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */
static void other_7F_opcodes(void)  { }

static void illegal(void)	{ M_ILLEGAL(); }
static void abst(void)
		{
			if (R.ACC >= 0x80000000) {
				R.ACC = ~R.ACC;
				R.ACC++ ;
				if (OVM && (R.ACC == 0x80000000)) R.ACC-- ;
			}
		}

/* ** The manual does not mention overflow with the ADD? instructions *****
   ** however i implelemted overflow, coz it doesnt make sense otherwise **
   ** and newer generations of this type of chip supported it. I think ****
   ** the manual is wrong (apart from other errors the manual has). *******

static void add_sh(void)	{ getdata(opcode_major,1); R.ACC += R.ALU; }
static void addh(void)		{ getdata(0,0); R.ACC += (R.ALU << 16); }
*/

static void add_sh(void)
		{
			tmpacc = R.ACC;
			getdata(opcode_major,1);
			R.ACC += R.ALU;
			if (tmpacc > R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x7fffffff;
			}
			else CLR(OV_FLAG);
		}
static void addh(void)
		{
			tmpacc = R.ACC;
			getdata(0,0);
			R.ACC += (R.ALU << 16);
			R.ACC &= 0xffff0000;
			R.ACC += (tmpacc & 0x0000ffff);
			if (tmpacc > R.ACC) {
				SET(OV_FLAG);
				if (OVM) {
					R.ACC &= 0x0000ffff; R.ACC |= 0x7fff0000;
				}
			}
			else CLR(OV_FLAG);
		}
static void adds(void)
		{
			tmpacc = R.ACC;
			getdata(0,0);
			R.ACC += R.ALU;
			if (tmpacc > R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x7fffffff;
			}
			else CLR(OV_FLAG);
		}
static void andopcode(void)
		{
			getdata(0,0);
			R.ACC &= R.ALU;
			R.ACC &= 0x0000ffff;
		}
static void apac(void)
		{
			tmpacc = R.ACC;
			R.ACC += R.Preg;
			if (tmpacc > R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x7fffffff;
			}
			else CLR(OV_FLAG);
		}
static void br(void)		{ R.PC = M_RDOP_ARG(R.PC); }
static void banz(void)
		{
			if ((R.AR[ARP] & 0x01ff) == 0) R.PC++ ;
			else R.PC = M_RDOP_ARG(R.PC);
			R.ALU = R.AR[ARP]; R.ALU-- ;
			R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (R.ALU & 0x01ff);
		}
static void bgez(void)
		{
			if (R.ACC >= 0) R.PC = M_RDOP_ARG(R.PC);
			else R.PC++ ;
		}
static void bgz(void)
		{
			if (R.ACC >  0) R.PC = M_RDOP_ARG(R.PC);
			else R.PC++ ;
		}
static void bioz(void)
		{
			if (R.BIO_pending_irq) R.PC = M_RDOP_ARG(R.PC);
			else R.PC++ ;
		}
static void blez(void)
		{
			if (R.ACC <= 0) R.PC = M_RDOP_ARG(R.PC);
			else R.PC++ ;
		}
static void blz(void)
		{
			if (R.ACC <  0) R.PC = M_RDOP_ARG(R.PC);
			else R.PC++ ;
		}
static void bnz(void)
		{
			if (R.ACC != 0) R.PC = M_RDOP_ARG(R.PC);
			else R.PC++ ;
		}
static void bv(void)
		{
			if (OV) {
				R.PC = M_RDOP_ARG(R.PC);
				CLR(OV_FLAG);
			}
			else R.PC++ ;
		}
static void bz(void)
		{
			if (R.ACC == 0) R.PC = M_RDOP_ARG(R.PC);
			else R.PC++ ;
		}
static void cala(void)
		{
			R.PC++ ;
			R.STACK[0] = R.STACK[1];
			R.STACK[1] = R.STACK[2];
			R.STACK[2] = R.STACK[3];
			R.STACK[3] = R.PC & ADDR_MASK;
			R.PC = R.ACC & ADDR_MASK;
		}
static void call(void)
		{
			R.PC++ ;
			R.STACK[0] = R.STACK[1];
			R.STACK[1] = R.STACK[2];
			R.STACK[2] = R.STACK[3];
			R.STACK[3] = R.PC & ADDR_MASK;
			R.PC = M_RDOP_ARG((R.PC-1)) & ADDR_MASK;
		}
static void dint(void)		{ SET(INTM_FLAG); }
static void dmov(void)		{ getdata(0,0); M_WRTRAM((memaccess+1),R.ALU); }
static void eint(void)		{ CLR(INTM_FLAG); }
static void in_p(void)
		{
			R.ALU = M_IN((opcode_major & 7));
			putdata((R.ALU & 0x0000ffff));
		}
static void lac_sh(void)
		{
			getdata((opcode_major & 0x0f),1);
			R.ACC = R.ALU;
		}
static void lack(void)		{ R.ACC = (opcode_minor & 0x000000ff); }
static void lar_ar0(void)	{ getdata_lar(); R.AR[0] = R.ALU; }
static void lar_ar1(void)	{ getdata_lar(); R.AR[1] = R.ALU; }
static void lark_ar0(void)	{ R.AR[0] = (opcode_minor & 0x00ff); }
static void lark_ar1(void)	{ R.AR[1] = (opcode_minor & 0x00ff); }
static void larp_mar(void)
		{
			if (opcode_minor & 0x80) {
				if ((opcode_minor & 0x20) || (opcode_minor & 0x10)) {
					UINT16 tmpAR = R.AR[ARP];
					if (opcode_minor & 0x20) tmpAR++ ;
					if (opcode_minor & 0x10) tmpAR-- ;
					R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
				}
				if (~opcode_minor & 0x08) {
					if (opcode_minor & 0x01) SET(ARP_REG) ;
					else CLR(ARP_REG);
				}
			}
		}
static void ldp(void)
		{
			getdata(0,0);
			if (R.ALU & 1) SET(DP_REG);
			else CLR(DP_REG);
		}
static void ldpk(void)
		{
			if (opcode_minor & 1) SET(DP_REG);
			else CLR(DP_REG);
		}
static void lst(void)
		{
			tmpacc = R.STR;
			opcode_minor |= 0x08; /* This dont support next arp, so make sure it dont happen */
			getdata(0,0);
			R.STR = R.ALU;
			tmpacc &= INTM_FLAG;
			R.STR |= tmpacc;
			R.STR |= 0x1efe;
		}
static void lt(void)		{ getdata(0,0); R.Treg = R.ALU; }
static void lta(void)
		{
			tmpacc = R.ACC;
			getdata(0,0);
			R.Treg = R.ALU;
			R.ACC += R.Preg;
			if (tmpacc > R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x7fffffff;
			}
			else CLR(OV_FLAG);
		}
static void ltd(void)
		{
			tmpacc = R.ACC;
			getdata(0,0);
			R.Treg = R.ALU;
			R.ACC += R.Preg;
			if (tmpacc > R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x7fffffff;
			}
			else CLR(OV_FLAG);
			M_WRTRAM((memaccess+1),R.ALU);
		}
static void mpy(void)
		{
			getdata(0,0);
			if ((R.ALU == 0x00008000) && (R.Treg == 0x8000))
				R.Preg = 0xc0000000;
			else R.Preg = R.ALU * R.Treg;
		}
static void mpyk(void)
		{
			if (opcode & 0x1000)
				R.Preg = R.Treg * ((opcode & 0x1fff) | 0xe000);
			else R.Preg = R.Treg * (opcode & 0x1fff);
		}
static void nop(void)		{ }
static void oropcode(void)
		{
			getdata(0,0);
			R.ALU &= 0x0000ffff;
			R.ACC |= R.ALU;
		}
static void out_p(void)
		{
			getdata(0,0);
			M_OUT((opcode_major & 7), (R.ALU & 0x0000ffff));
		}
static void pac(void)		{ R.ACC = R.Preg; }
static void pop(void)
		{
			R.ACC = R.STACK[3] & ADDR_MASK;
			R.STACK[3] = R.STACK[2];
			R.STACK[2] = R.STACK[1];
			R.STACK[1] = R.STACK[0];
		}
static void push(void)
		{
			R.STACK[0] = R.STACK[1];
			R.STACK[1] = R.STACK[2];
			R.STACK[2] = R.STACK[3];
			R.STACK[3] = R.ACC & ADDR_MASK;
		}
static void ret(void)
		{
			R.PC = R.STACK[3] & ADDR_MASK;
			R.STACK[3] = R.STACK[2];
			R.STACK[2] = R.STACK[1];
			R.STACK[1] = R.STACK[0];
		}
static void rovm(void)		{ CLR(OVM_FLAG); }
static void sach_sh(void)	{ putdata(((R.ACC << (opcode_major & 7)) >> 16)); }
static void sacl(void)		{ putdata((R.ACC & 0x0000ffff)); }
static void sar_ar0(void)	{ putdata(0); }
static void sar_ar1(void)	{ putdata(1); }
static void sovm(void)		{ SET(OVM_FLAG); }
static void spac(void)
		{
			INT32 tmpPreg = R.Preg;
			tmpacc = R.ACC ;
			/* if (tmpPreg & 0x8000) tmpPreg |= 0xffff0000; */
			R.ACC -= tmpPreg ;
			if (tmpacc < R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x80000000;
			}
			else CLR(OV_FLAG);
		}
static void sst(void)		{ putdata_sst(R.STR); }
static void sub_sh(void)
		{
			tmpacc = R.ACC;
			getdata((opcode_major & 0x0f),1);
			R.ACC -= R.ALU;
			if (tmpacc < R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x80000000;
			}
			else CLR(OV_FLAG);
		}
static void subc(void)
		{
			tmpacc = R.ACC;
			getdata(15,0);
			tmpacc -= R.ALU;
			if (tmpacc < 0) {
				R.ACC <<= 1;
				SET(OV_FLAG);
			}
			else R.ACC = ((tmpacc << 1) + 1);
		}
static void subh(void)
		{
			tmpacc = R.ACC;
			getdata(0,0);
			R.ACC -= (R.ALU << 16);
			R.ACC &= 0xffff0000;
			R.ACC += (tmpacc & 0x0000ffff);
			if ((tmpacc & 0xffff0000) < (R.ACC & 0xffff0000)) {
				SET(OV_FLAG);
				if (OVM) {
					R.ACC = (tmpacc & 0x0000ffff);
					R.ACC |= 0x80000000 ;
				}
			}
			else CLR(OV_FLAG);
		}
static void subs(void)
		{
			tmpacc = R.ACC;
			getdata(0,0);
			R.ACC -= R.ALU;
			if (tmpacc < R.ACC) {
				SET(OV_FLAG);
				if (OVM) R.ACC = 0x80000000;
			}
			else CLR(OV_FLAG);
		}
static void tblr(void)
		{
			R.ALU = M_RDROM((R.ACC & ADDR_MASK));
			putdata(R.ALU);
			R.STACK[0] = R.STACK[1];
		}
static void tblw(void)
		{
			getdata(0,0);
			M_WRTROM(((R.ACC & ADDR_MASK)),R.ALU);
			R.STACK[0] = R.STACK[1];
		}
static void xoropcode(void)
		{
			tmpacc = (R.ACC & 0xffff0000);
			getdata(0,0);
			R.ACC ^= R.ALU;
			R.ACC &= 0x0000ffff;
			R.ACC |= tmpacc;
		}
static void zac(void)		{ R.ACC = 0; }
static void zalh(void)		{ getdata(16,0); R.ACC = R.ALU; }
static void zals(void)		{ getdata(0 ,0); R.ACC = R.ALU; }


static unsigned cycles_main[256]=
{
/*00*/		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*10*/		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*20*/		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*30*/		1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
/*40*/		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/*50*/		1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
/*60*/		1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1,
/*70*/		1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 3, 1, 0,
/*80*/		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*90*/		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*A0*/		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*B0*/		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*C0*/		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*D0*/		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*E0*/		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*F0*/		0, 0, 0, 0, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2
};

static unsigned cycles_7F_other[32]=
{
/*80*/		1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 1, 1,
/*90*/		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1,
};

static opcode_fn opcode_main[256]=
{
/*00*/  add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh
/*08*/ ,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh
/*10*/ ,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh
/*18*/ ,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh
/*20*/ ,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh
/*28*/ ,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh
/*30*/ ,sar_ar0		,sar_ar1	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*38*/ ,lar_ar0		,lar_ar1	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*40*/ ,in_p		,in_p		,in_p		,in_p		,in_p		,in_p		,in_p		,in_p
/*48*/ ,out_p		,out_p		,out_p		,out_p		,out_p		,out_p		,out_p		,out_p
/*50*/ ,sacl		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*58*/ ,sach_sh		,sach_sh	,sach_sh	,sach_sh	,sach_sh	,sach_sh	,sach_sh	,sach_sh
/*60*/ ,addh		,adds		,subh		,subs		,subc		,zalh		,zals		,tblr
/*68*/ ,larp_mar	,dmov		,lt			,ltd		,lta		,mpy		,ldpk		,ldp
/*70*/ ,lark_ar0	,lark_ar1	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*78*/ ,xoropcode		,andopcode	,oropcode		,lst		,sst		,tblw		,lack		,other_7F_opcodes
/*80*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*88*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*90*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*98*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*A0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*A8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*B0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*B8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*C0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*C8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*D0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*D8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*E0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*E8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*F0*/ ,illegal		,illegal	,illegal	,illegal	,banz		,bv			,bioz		,illegal
/*F8*/ ,call		,br			,blz		,blez		,bgz		,bgez		,bnz		,bz
};

static opcode_fn opcode_7F_other[32]=
{
/*80*/  nop			,dint		,eint		,illegal	,illegal	,illegal	,illegal	,illegal
/*88*/ ,abst		,zac		,rovm		,sovm		,cala		,ret		,pac		,apac
/*90*/ ,spac		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*98*/ ,illegal		,illegal	,illegal	,illegal	,push		,pop		,illegal	,illegal
};



/****************************************************************************/
/* Set all registers to given values										*/
/****************************************************************************/
void TMS320C10_SetRegs (TMS320C10_Regs *Regs)
{
	R=*Regs;
}

/****************************************************************************/
/* Get all registers in given buffer										*/
/****************************************************************************/
void TMS320C10_GetRegs (TMS320C10_Regs *Regs)
{
	*Regs=R;
}

/****************************************************************************/
/* Reset registers to their initial values									*/
/****************************************************************************/
void TMS320C10_Reset (void)
{
	R.PC  = 0;
	R.STR  = 0x0fefe;
	R.ACC = 0;
	R.pending_irq		= TMS320C10_NOT_PENDING;
	R.BIO_pending_irq	= TMS320C10_NOT_PENDING;
}


/****************************************************************************/
/* Return program counter													*/
/****************************************************************************/
unsigned TMS320C10_GetPC (void)
{
	return R.PC;
}


/****************************************************************************/
/* Issue an interrupt if necessary											*/
/****************************************************************************/

static int Ext_IRQ(void)
{
	if (INTM == 0)
	{
		SET(INTM_FLAG);
		R.STACK[0] = R.STACK[1];
		R.STACK[1] = R.STACK[2];
		R.STACK[2] = R.STACK[3];
		R.STACK[3] = R.PC & ADDR_MASK;
		R.PC = 0x0002;
		R.pending_irq = TMS320C10_NOT_PENDING;
		return 3;  /* 3 clock cycles used due to PUSH and DINT operation ? */
	}
	return 0;
}




/****************************************************************************/
/* Execute IPeriod. Return 0 if emulation should be stopped					*/
/****************************************************************************/
int TMS320C10_Execute(int cycles)
{
	TMS320C10_ICount=cycles;

	do
	{

#if NEW_INTERRUPT_SYSTEM
		if (R.pending_irq & TMS320C10_PENDING) {
			int type = (*R.irq_callback)(0);
			R.pending_irq |= type;
		}
#endif

		if (R.pending_irq) {
			/* Dont service INT if prev instruction was MPY, MPYK or EINT */
			if ((opcode_major != 0x6d) || ((opcode_major & 0xe0) != 0x80) || (opcode != 0x7f82))
				TMS320C10_ICount -= Ext_IRQ();
		}

		#ifdef MAME_DEBUG
		{
			extern int mame_debug;
			if (mame_debug) MAME_Debug();
		}
		#endif

		opcode=M_RDOP(R.PC);
		opcode_major = ((opcode & 0x0ff00) >> 8);
		opcode_minor = (opcode & 0x0ff);

		{
		extern int previouspc;
		previouspc = R.PC;
		}

		R.PC++;
		if (opcode_major != 0x07f) { /* Do all opcodes except the 7Fxx ones */
			TMS320C10_ICount -= cycles_main[opcode_major];
			(*(opcode_main[opcode_major]))();
		}
		else { /* Opcode major byte 7Fxx has many opcodes in its minor byte */
			opcode_minr = (opcode & 0x001f);
			TMS320C10_ICount -= cycles_7F_other[opcode_minr];
			(*(opcode_7F_other[opcode_minr]))();
		}
	}
	while (TMS320C10_ICount>0);

	return cycles - TMS320C10_ICount;
}

#if NEW_INTERRUPT_SYSTEM

void TMS320C10_set_nmi_line(int state)
{
	/* TMS320C10 does not have a NMI line */
}

void TMS320C10_set_irq_line(int irqline, int state)
{
	if (irqline == TMS320C10_ACTIVE_INT) {
		R.irq_state = state;
		if (state == CLEAR_LINE) R.pending_irq &= ~TMS320C10_PENDING;
		if (state == ASSERT_LINE) R.pending_irq |= TMS320C10_PENDING;
	}
	if (irqline == TMS320C10_ACTIVE_BIO) {
		if (state == CLEAR_LINE) R.BIO_pending_irq &= ~TMS320C10_PENDING;
		if (state == ASSERT_LINE) R.BIO_pending_irq |= TMS320C10_PENDING;
	}
}

void TMS320C10_set_irq_callback(int (*callback)(int irqline))
{
	R.irq_callback = callback;
}

#else

void TMS320C10_Cause_Interrupts(int type)
{
	switch (type) {
		case TMS320C10_ACTIVE_INT:	R.pending_irq = TMS320C10_PENDING; break;
		case TMS320C10_ACTIVE_BIO:	R.BIO_pending_irq = TMS320C10_PENDING; break;
		case TMS320C10_IGNORE_BIO:	/* Cant turn off individual interrupts in   */
									/* clear function below, so support it here */
									R.BIO_pending_irq = TMS320C10_NOT_PENDING; break;
	}
}

void TMS320C10_Clear_Pending_Interrupts(void)
{
	R.pending_irq = TMS320C10_NOT_PENDING;
}

#endif
