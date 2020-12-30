#ifndef DAC_H
#define DAC_H

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

#define MAX_DAC 4

struct DACinterface
{
	int num;	/* total number of DACs */
	int mixing_level[MAX_DAC];
};

int DAC_sh_start(struct DACinterface *interface);
void DAC_data_w(int num,int data);
void DAC_0_data_w(int num,int data);
void DAC_1_data_w(int num,int data);
void DAC_signed_data_w(int num,int data);
void DAC_data_16_w(int num,int data);
void DAC_signed_data_16_w(int num,int data);
void DAC_sh_stop(void);
void DAC_sh_update(void);
void DAC_set_volume(int num,int volume,int gain);
#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif
#endif
