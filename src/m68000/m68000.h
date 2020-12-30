#ifndef M68000_H
#define M68000_H
#include "cpudefs.h"

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

/* Assembler core - m68k_regs in Assembler routine */
#ifdef A68KEM
extern m68k_regstruct m68k_regs;
#endif

/* ASG 971105 */
typedef struct
{
	m68k_regstruct m68k_regs;
	int pending_interrupts;
} MC68000_Regs;

extern void MC68000_Reset(void);                      /* ASG 971105 */
extern int  MC68000_Execute(int);                     /* ASG 971105 */

#if defined(__cplusplus) && !defined(USE_CYCLONE)
extern "C" {
#endif
extern void MC68000_SetRegs(MC68000_Regs *);          /* ASG 971105 */
extern void MC68000_GetRegs(MC68000_Regs *);          /* ASG 971105 */
#if defined(__cplusplus) && !defined(USE_CYCLONE)
}
#endif

extern void MC68000_Cause_Interrupt(int);             /* ASG 971105 */
extern void MC68000_Clear_Pending_Interrupts(void);   /* ASG 971105 */
extern int  MC68000_GetPC(void);                      /* ASG 971105 */


#ifdef USE_CYCLONE
#include "../cyclone/m68000.h"
#else
#ifdef USE_FAME
#ifdef FAME_IS_FAMEC
#define FAME_IO_CYCLE io_cycle_counter
#else
#ifdef DREAMCAST
#define FAME_IO_CYCLE _io_cycle_counter
#else
#define FAME_IO_CYCLE __io_cycle_counter
#endif
#endif
#define MC68000_ICount FAME_IO_CYCLE
extern int MC68000_ICount;
#else
extern int MC68000_ICount;                            /* ASG 971105 */
#endif
#endif


#define MC68000_INT_NONE 0							  /* ASG 971105 */
#define MC68000_IRQ_1    1
#define MC68000_IRQ_2    2
#define MC68000_IRQ_3    3
#define MC68000_IRQ_4    4
#define MC68000_IRQ_5    5
#define MC68000_IRQ_6    6
#define MC68000_IRQ_7    7

#define MC68000_STOP     0x10

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif

#endif
