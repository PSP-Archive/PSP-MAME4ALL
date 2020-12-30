#ifndef __YM2151INTF_H
#define __YM2151INTF_H

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

#define MAX_2151 3

#ifndef VOL_YM3012
#define YM3012_VOL(LVol,LPan,RVol,RPan) ((LVol)|((LPan)<<8)|((RVol)<<16)|((RPan)<<24))
#endif

struct YM2151interface
{
	int num;
	int baseclock;
	int volume[MAX_2151];
	void (*irqhandler[MAX_2151])(void);
	void (*portwritehandler[MAX_2151])(int,int);
};

#define YM2151_STEREO_REVERSE 0x100

int YM2151_status_port_0_r(int offset);
int YM2151_status_port_1_r(int offset);
int YM2151_status_port_2_r(int offset);

void YM2151_register_port_0_w(int offset,int data);
void YM2151_register_port_1_w(int offset,int data);
void YM2151_register_port_2_w(int offset,int data);

void YM2151_data_port_0_w(int offset,int data);
void YM2151_data_port_1_w(int offset,int data);
void YM2151_data_port_2_w(int offset,int data);
int YM2151_sh_start(struct YM2151interface *interface,int mode);
void YM2151_sh_stop(void);
void YM2151_sh_update(void);

void YM2151UpdateRequest(int chip);

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif
#endif
