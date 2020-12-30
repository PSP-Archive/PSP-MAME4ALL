
#ifndef ASTROCADE_H
#define ASTROCADE_H

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif
#define MAX_ASTROCADE_CHIPS 2   /* max number of emulated chips */

struct astrocade_interface
{
	int num;			/* total number of sound chips in the machine */
	int baseclock;			/* astrocade clock rate  */
	int volume;			/* master volume */
};

int astrocade_sh_start(struct astrocade_interface *interface);
void astrocade_sh_stop(void);
void astrocade_sh_update(void);

void astrocade_sound1_w(int offset, int data);
void astrocade_sound2_w(int offset, int data);

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif
#endif
