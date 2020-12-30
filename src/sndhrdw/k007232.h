/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/
#ifndef __KDAC_A_H__
#define __KDAC_A_H__

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

struct K007232_interface
{
	int bankA, bankB;	/* memory regions for channel A and B */
	int volume;
	int limit;
};


int K007232_sh_start(struct K007232_interface *intf);
void K007232_sh_stop(void);
void K007232_sh_update(void);
void K007232_WriteReg(int r,int v);
int K007232_ReadReg(int r);

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif
#endif
