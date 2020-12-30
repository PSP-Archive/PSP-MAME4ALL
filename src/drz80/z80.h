#ifndef Z80_DRZ80_H
#define Z80_DRZ80_H

#include "osd_cpu.h"
#include "cpuintrf.h"
#include "drz80.h"

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

typedef struct DrZ80 Z80_Regs;
extern Z80_Regs Z80;
#define Z80_ICount (Z80.cycles)

#define Z80_IGNORE_INT	-1 /* Ignore interrupt */
#define Z80_NMI_INT 	-2 /* Execute NMI */
#define Z80_IRQ_INT 	-1000 /* Execute IRQ */

extern unsigned Z80_GetPC (void); /* Get program counter */
extern void Z80_GetRegs (Z80_Regs *Regs);  /* Get registers */
extern void Z80_SetRegs (Z80_Regs *Regs);  /* Set registers */
extern void Z80_Reset (Z80_DaisyChain *daisy_chain);
extern int Z80_Execute(int cycles); /* Execute cycles T-States - returns number of cycles actually run */
extern int Z80_Interrupt(void);
extern void Z80_Cause_Interrupt(int type);
extern void Z80_Clear_Pending_Interrupts(void);

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern void Interrupt(void); /* required for DrZ80 int hack */
#ifdef __cplusplus
} /* End of extern "C" */
#endif

#endif

