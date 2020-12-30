#ifndef M68000_H
#define M68000_H

#include "fame.h"

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

/*
typedef struct
{
	struct M68K_CONTEXT;
	int pending_interrupts;
} MC68000_Regs;
*/

typedef struct M68K_CONTEXT MC68000_Regs;

extern void MC68000_Reset(void);                      
extern int  MC68000_Execute(int);
#ifdef __cplusplus
extern "C" {
#endif
extern void MC68000_SetRegs(MC68000_Regs *);          
extern void MC68000_GetRegs(MC68000_Regs *);          
#ifdef __cplusplus
}
#endif
extern void MC68000_Cause_Interrupt(int);             
extern void MC68000_Clear_Pending_Interrupts(void);   
extern int  MC68000_GetPC(void);                      

#ifdef FAME_IS_FAMEC
#define FAME_IO_CYCLE io_cycle_counter
#else
#ifdef DREAMCAST
#define FAME_IO_CYCLE _io_cycle_counter
#else
#define FAME_IO_CYCLE __io_cycle_counter
#endif
#endif

extern int FAME_IO_CYCLE;

#define MC68000_ICount FAME_IO_CYCLE

#define MC68000_INT_NONE 0							  
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
