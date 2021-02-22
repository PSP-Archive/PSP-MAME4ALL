#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "driver.h"
#include "cpuintrf.h"
#include "memory.h"
#include "z80/z80.h"
#include "raze/raze/raze.h"

int Z80_ICount;

#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)

#ifdef __cplusplus
extern "C" {
#endif

typedef UWORD (*z80_get_reg_func)(z80_register);
typedef int (*z80_emulate_func)(int);
typedef void (*z80_set_reg_func)(z80_register,UWORD);
typedef void (*z80_init_memmap_func)(void);
typedef void (*z80_map_fetch_func)(UWORD,UWORD,UBYTE *);
typedef void (*z80_add_read_func)(UWORD,UWORD,int,void *);
typedef void (*z80_add_write_func)(UWORD,UWORD,int,void *);
typedef void (*z80_set_in_func)(UBYTE (*handler)(UWORD port));
typedef void (*z80_set_out_func)(void (*handler)(UWORD port, UBYTE value));
typedef void (*z80_reset_func)(void);
typedef void (*z80_raise_IRQ_func)(UBYTE);
typedef void (*z80_lower_IRQ_func)(void);
typedef void (*z80_cause_NMI_func)(void);
typedef void (*z80_end_memmap_func)(void);

typedef struct{
	z80_get_reg_func get_reg;
	z80_emulate_func emulate;
	z80_set_reg_func set_reg;
	z80_init_memmap_func init_memmap;
	z80_map_fetch_func map_fetch;
	z80_add_read_func add_read;
	z80_add_write_func add_write;
	z80_set_in_func set_in;
	z80_set_out_func set_out;
	z80_reset_func reset;
	z80_raise_IRQ_func raise_IRQ;
	z80_lower_IRQ_func lower_IRQ;
	z80_cause_NMI_func cause_NMI;
	z80_end_memmap_func end_memmap;
}z80_struct_t;

#define Z80_STRUCT_FUNCS(NAME) \
UWORD z80_get_reg_##NAME (z80_register reg); \
int   z80_emulate_##NAME (int cycles); \
void  z80_set_reg_##NAME (z80_register reg, UWORD value); \
void  z80_init_memmap_##NAME (void); \
void  z80_map_fetch_##NAME (UWORD start, UWORD end, UBYTE *memory); \
void  z80_add_read_##NAME (UWORD start, UWORD end, int method, void *data); \
void  z80_add_write_##NAME (UWORD start, UWORD end, int method, void *data); \
void  z80_set_in_##NAME (UBYTE (*handler)(UWORD port)); \
void  z80_set_out_##NAME (void (*handler)(UWORD port, UBYTE value)); \
void  z80_reset_##NAME (void); \
void  z80_raise_IRQ_##NAME (UBYTE vector); \
void  z80_lower_IRQ_##NAME (void); \
void  z80_cause_NMI_##NAME (void); \
void  z80_end_memmap_##NAME (void); \
static z80_struct_t z80_struct_##NAME = { \
	z80_get_reg_##NAME , z80_emulate_##NAME , z80_set_reg_##NAME , z80_init_memmap_##NAME , z80_map_fetch_##NAME , z80_add_read_##NAME , z80_add_write_##NAME , z80_set_in_##NAME , z80_set_out_##NAME , z80_reset_##NAME , z80_raise_IRQ_##NAME , z80_lower_IRQ_##NAME , z80_cause_NMI_##NAME , z80_end_memmap_##NAME };


Z80_STRUCT_FUNCS(1)
#if MAX_CPU > 1
Z80_STRUCT_FUNCS(2)
#if MAX_CPU > 2
Z80_STRUCT_FUNCS(3)
#if MAX_CPU > 3
Z80_STRUCT_FUNCS(4)
#if MAX_CPU > 4
Z80_STRUCT_FUNCS(5)
#if MAX_CPU > 5
Z80_STRUCT_FUNCS(6)
#if MAX_CPU > 6 
Z80_STRUCT_FUNCS(7)
#if MAX_CPU > 7 
Z80_STRUCT_FUNCS(8)
#endif
#endif
#endif
#endif
#endif
#endif
#endif

static z80_struct_t z80s[MAX_CPU]= {
	z80_struct_1,
#if MAX_CPU > 1
	z80_struct_2,
#if MAX_CPU > 2
	z80_struct_3,
#if MAX_CPU > 3
	z80_struct_4,
#if MAX_CPU > 4
	z80_struct_5,
#if MAX_CPU > 5
	z80_struct_6,
#if MAX_CPU > 6 
	z80_struct_7,
#if MAX_CPU > 7 
	z80_struct_8,
#endif
#endif
#endif
#endif
#endif
#endif
#endif
};

static z80_struct_t *z80=(z80_struct_t *)&z80s[0];

#define z80_get_reg(A) z80->get_reg(A)
#define z80_emulate(A) z80->emulate(A)
#define z80_set_reg(A,B) z80->set_reg(A,B)
#define z80_init_memmap() z80->init_memmap()
#define z80_map_fetch(A,B,C) z80->map_fetch(A,B,C)
#define z80_add_read(A,B,C,D) z80->add_read(A,B,C,D)
#define z80_add_write(A,B,C,D) z80->add_write(A,B,C,D)
#define z80_set_in(A) z80->set_in(A)
#define z80_set_out(A) z80->set_out(A)
#define z80_reset() z80->reset()
#define z80_raise_IRQ(A) z80->raise_IRQ(A)
#define z80_lower_IRQ() z80->lower_IRQ()
#define z80_cause_NMI() z80->cause_NMI()
#define z80_end_memmap() z80->end_memmap()

void mame_change_pc16(unsigned newpc)
{
	register unsigned short pc=(unsigned short)(newpc&0xFFFF);
	change_pc16((pc));
}

#ifdef __cplusplus
};
#endif

#endif


#ifndef RAZE_ONLY_HANDLER
extern unsigned char *RAM;
extern MHELE readhardware[MH_ELEMAX << MH_SBITS];  /* mem/port read  */
extern MHELE writehardware[MH_ELEMAX << MH_SBITS]; /* mem/port write */
extern int (*memoryreadhandler[MH_HARDMAX])(int address);
extern void (*memorywritehandler[MH_HARDMAX])(int address,int data);
extern int memoryreadoffset[MH_HARDMAX];
extern int memorywriteoffset[MH_HARDMAX];

static int check_readmem16 (int address)
{
	MHELE hw;
	unsigned ret;

	/* 1st element link */
	hw = cur_mrhard[address >> (ABITS2_16 + ABITS_MIN_16)];
	if (!hw) return 0;
	if (hw >= MH_HARDMAX)
	{
		/* 2nd element link */
		hw = readhardware[((hw - MH_HARDMAX) << MH_SBITS) + ((address >> ABITS_MIN_16) & MHMASK(ABITS2_16))];
		if (!hw) return 0;
	}

	/* fallback to handler */
	ret=(unsigned)memoryreadhandler[hw];
	if (!ret || memoryreadoffset[hw])
		return -1;
	return ret;
}

static int check_read_block(int address)
{
	int i, ret=check_readmem16(address);
	for(i=1;i<256;i++)
		if (ret!=check_readmem16(address+i))
				return -1;
	return ret;
}

static void get_read_map(void)
{
	int i;
	for(i=0;i<0x10000;i+=0x100)
	{
		int blck=check_read_block(i);
		switch(blck)
		{
			case 0:
				z80_add_read(i, i+0xFF, Z80_MAP_DIRECT,(void *)(((unsigned)&RAM[i])));
				break;
			case -1:
				z80_add_read(i, i+0xFF, Z80_MAP_HANDLED,(void *)&cpu_readmem16);
				break;
			default:
				z80_add_read(i, i+0xFF, Z80_MAP_HANDLED,(void *)blck);
		}
	}
}


static int check_writemem16 (int address)
{
	MHELE hw;
	int ret;

	/* 1st element link */
	hw = cur_mwhard[address >> (ABITS2_16 + ABITS_MIN_16)];
	if (!hw)
		return 0;
	if (hw >= MH_HARDMAX)
	{
		/* 2nd element link */
		hw = writehardware[((hw - MH_HARDMAX) << MH_SBITS) + ((address >> ABITS_MIN_16) & MHMASK(ABITS2_16))];
		if (!hw)
			return 0;
	}

	/* fallback to handler */
	ret=(int)memorywritehandler[hw];
	if (!ret || memorywriteoffset[hw])
		return -1;
	return ret;
}


static int check_write_block(int address)
{
	int i, ret=check_writemem16(address);
	for(i=1;i<256;i++)
		if (ret!=check_writemem16(address+i))
				return -1;
	return ret;
}

static void get_write_map(void)
{
	int i;
	for(i=0;i<0x10000;i+=0x100)
	{
		int blck=check_write_block(i);
		switch(blck)
		{
			case 0:
				z80_add_write(i, i+0xFF, Z80_MAP_DIRECT,(void *)(((unsigned)&RAM[i])));
				break;
			case -1:
				z80_add_write(i, i+0xFF, Z80_MAP_HANDLED,(void *)&cpu_writemem16);
				break;
			default:
				z80_add_write(i, i+0xFF, Z80_MAP_HANDLED,(void *)blck);
		}
	}
}

#endif


#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
extern Z80_Regs mameZ80;
void mameZ80_Reset(Z80_DaisyChain *daisy_chain);
void mameZ80_GetRegs(Z80_Regs *Regs);
void mameZ80_SetRegs (Z80_Regs *Regs);
unsigned mameZ80_GetPC (void);
unsigned mameZ80_GetSP (void);
void mameZ80_Cause_Interrupt(int);
void mameZ80_Clear_Pending_Interrupts(void);
int mameZ80_Execute(int);
extern struct GameDriver gorf_driver;
extern struct GameDriver gorfpgm1_driver;
extern struct GameDriver wow_driver;
extern struct GameDriver bosco_driver;
extern struct GameDriver bosconm_driver;
extern struct GameDriver missile_driver;
extern struct GameDriver missile2_driver;
extern struct GameDriver suprmatk_driver;
extern struct GameDriver btime_driver;
extern struct GameDriver btimea_driver;
extern struct GameDriver cookrace_driver;
extern struct GameDriver eggs_driver;
extern struct GameDriver scregg_driver;
extern struct GameDriver lnc_driver;
extern struct GameDriver bnj_driver;
extern struct GameDriver brubber_driver;
extern struct GameDriver caractn_driver;
extern struct GameDriver zoar_driver;
extern struct GameDriver berzerk_driver;
extern struct GameDriver berzerk1_driver;
extern struct GameDriver frenzy_driver;
extern struct GameDriver frenzy1_driver;
extern struct GameDriver maketrax_driver;
extern struct GameDriver spacfury_driver;
extern struct GameDriver spacfura_driver;
extern struct GameDriver zektor_driver;
extern struct GameDriver tacscan_driver;
extern struct GameDriver elim2_driver;
extern struct GameDriver elim4_driver;
extern struct GameDriver startrek_driver;
extern struct GameDriver astrob_driver;
extern struct GameDriver astrob1_driver;
extern struct GameDriver spaceod_driver;
extern struct GameDriver s005_driver;
extern struct GameDriver monsterb_driver;
extern struct GameDriver pignewta_driver;
extern struct GameDriver pignewt_driver;
extern struct GameDriver starwars_driver;
extern struct GameDriver empire_driver;
extern struct GameDriver tempest_driver;
extern struct GameDriver tempest1_driver;
extern struct GameDriver tempest2_driver;
extern struct GameDriver temptube_driver;
extern struct GameDriver spyhunt_driver;
extern struct GameDriver rampage_driver;
extern struct GameDriver powerdrv_driver;
extern struct GameDriver maxrpm_driver;
extern struct GameDriver sarge_driver;
extern struct GameDriver dotron_driver;
extern struct GameDriver dotrone_driver;
#endif

void Z80_Reset(Z80_DaisyChain *daisy_chain)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	int forced=0
#ifndef NO_DRIVER_GENERIC_8BIT
	 || (Machine->gamedrv == (const struct GameDriver *) &wow_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &gorf_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &gorfpgm1_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &bosco_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &bosconm_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &missile_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &missile2_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &suprmatk_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &btime_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &btimea_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &cookrace_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &eggs_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &scregg_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &lnc_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &bnj_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &brubber_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &caractn_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &zoar_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &berzerk_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &berzerk1_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &frenzy_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &frenzy1_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &maketrax_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &spacfury_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &spacfura_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &zektor_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &tacscan_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &elim2_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &elim4_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &startrek_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &astrob_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &astrob1_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &spaceod_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &s005_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &monsterb_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &pignewta_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &pignewt_driver)
#endif
#ifndef NO_DRIVER_ATARIVG
	 || (Machine->gamedrv == (const struct GameDriver *) &starwars_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &empire_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &tempest_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &tempest1_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &tempest2_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &temptube_driver)
#endif
#ifndef NO_DRIVER_MCR
	 || (Machine->gamedrv == (const struct GameDriver *) &spyhunt_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &rampage_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &powerdrv_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &maxrpm_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &sarge_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &dotron_driver)
	 || (Machine->gamedrv == (const struct GameDriver *) &dotrone_driver)
#endif
	;

	z80=(z80_struct_t *)&z80s[cpu_getactivecpu()];
printf("RAZE Z80_Reset %i\n",cpu_getactivecpu()); fflush(stdout);
	if (daisy_chain || forced)
	{
		z80=NULL;
		mameZ80_Reset(daisy_chain);
	}
	else
#endif
	{
	z80_init_memmap();
	z80_map_fetch(0x0000, 0xffff,OP_ROM);
#ifdef RAZE_ONLY_HANDLER
	z80_add_read(0x0000, 0xffff, Z80_MAP_HANDLED,(void *)&cpu_readmem16);
	z80_add_write(0x0000, 0xffff, Z80_MAP_HANDLED,(void *)&cpu_writemem16);
#else
	get_read_map();
	get_write_map();
#endif
	z80_end_memmap();
	z80_set_in((unsigned char (*)(short unsigned int))&cpu_readport);
	z80_set_out((void (*)(short unsigned int, unsigned char))&cpu_writeport);
	z80_reset();
	Z80_Clear_Pending_Interrupts();
	z80_set_reg(Z80_REG_SP,0xf000);
	change_pc16(z80_get_reg(Z80_REG_PC));
	}
}

void Z80_GetRegs(Z80_Regs *Regs)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (z80)
		*((z80_struct_t **)Regs)=z80;
	else
		mameZ80_GetRegs(Regs);
#endif
}

unsigned char Z80_GetB(void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80.BC.B.h;			
	else
#endif
	return z80_get_reg(Z80_REG_BC)>>8;
}

unsigned char Z80_GetD(void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80.DE.B.h;
	else
#endif
	return z80_get_reg(Z80_REG_DE)>>8;
}

unsigned char Z80_GetE(void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80.DE.B.l;
	else
#endif
	return z80_get_reg(Z80_REG_DE)&0xFF;
}

unsigned short Z80_GetHL(void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80.HL.D;
	else
#endif
	return z80_get_reg(Z80_REG_HL)&0xFFFF;
}

unsigned short Z80_GetHL2(void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80.HL2.D;
	else
#endif
	return z80_get_reg(Z80_REG_HL2)&0xFFFF;
}

unsigned short Z80_GetBC(void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80.BC.B.l;
	else
#endif
	return z80_get_reg(Z80_REG_BC)&0xFFFF;
}


void Z80_SetRegs (Z80_Regs *Regs)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	z80=*((z80_struct_t **)Regs);
	if (((unsigned)z80>=(unsigned)&z80s[0])&&((unsigned)z80<=(unsigned)&z80s[MAX_CPU]))
	{
		unsigned short pc=z80_get_reg(Z80_REG_PC);
		change_pc16(pc);
		z80_set_reg(Z80_REG_PC,pc);
	}
	else
	{
		z80=NULL;
		mameZ80_SetRegs(Regs);
	}
#endif
}

unsigned Z80_GetPC (void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80_GetPC();
	else
#endif
	return z80_get_reg(Z80_REG_PC);
}

unsigned Z80_GetSP (void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80_GetSP();
	else
#endif
	return z80_get_reg(Z80_REG_SP);
}


void Z80_Cause_Interrupt(int type)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		mameZ80_Cause_Interrupt(type);
	else
#endif
	{
	if (type == Z80_NMI_INT)
		z80_cause_NMI();
	else if (type >= 0)
		z80_raise_IRQ(type & 0xff);
	}
}

void Z80_Clear_Pending_Interrupts(void)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		mameZ80_Clear_Pending_Interrupts();
	else
#endif
	z80_lower_IRQ();
}

int Z80_Execute(int cycles)
{
#if defined(MAME4ALL_AGED) || defined(MAME4ALL_CLASSIC)
	if (!z80)
		return mameZ80_Execute(cycles);
	else
#endif
	return z80_emulate(cycles);
}

