#ifndef M68000_CYCLONE_H
#define M68000_CYCLONE_H
#include "cyclone.h"

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

extern struct Cyclone cyclone;
#define MC68000_ICount (cyclone.cycles)

#ifndef M68000_H

typedef struct
{
	struct Cyclone regs;
	int pending_interrupts;
} MC68000_Regs;

extern void MC68000_Reset(void);
extern int  MC68000_Execute(int);
extern void MC68000_SetRegs(MC68000_Regs *);
extern void MC68000_GetRegs(MC68000_Regs *);
extern void MC68000_Cause_Interrupt(int);
extern void MC68000_Clear_Pending_Interrupts(void);
extern int  MC68000_GetPC(void);

#endif

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif

#endif
