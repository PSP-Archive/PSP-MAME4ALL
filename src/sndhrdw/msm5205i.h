#ifndef MSM5205I_H
#define MSM5205I_H

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

/* an interface for the MSM5205I and similar chips */

#define MAX_MSM5205I 4

/* priscaler selector defines   */
/* default master clock is 384KHz */
#define MSM5205I_S96_3B 0     /* prsicaler 1/96(4KHz) , data 3bit */
#define MSM5205I_S48_3B 1     /* prsicaler 1/48(8KHz) , data 3bit */
#define MSM5205I_S64_3B 2     /* prsicaler 1/64(6KHz) , data 3bit */
#define MSM5205I_SEX_3B 3     /* VCLK slave mode      , data 3bit */
#define MSM5205I_S96_4B 4     /* prsicaler 1/96(4KHz) , data 4bit */
#define MSM5205I_S48_4B 5     /* prsicaler 1/48(8KHz) , data 4bit */
#define MSM5205I_S64_4B 6     /* prsicaler 1/64(6KHz) , data 4bit */
#define MSM5205I_SEX_4B 7     /* VCLK slave mode      , data 4bit */

struct MSM5205Iinterface
{
	int num;                       /* total number of chips                 */
	int baseclock;                 /* master clock (default = 384KHz)       */
	void (*vclk_interrupt[MAX_MSM5205I])(int);   /* VCLK interrupt callback  */
	int select[MAX_MSM5205I];       /* prescaler / bit width selector        */
	int mixing_level[MAX_MSM5205I]; /* master volume                         */
};

int MSM5205I_sh_start (const struct MSM5205Iinterface *intf);
void MSM5205I_sh_stop (void);   /* empty this function */
void MSM5205I_sh_update (void); /* empty this function */
void MSM5205I_sh_reset (void);

/* reset signal should keep for 2cycle of VCLK      */
void MSM5205I_reset_w (int num, int reset);
/* adpcmata is latched after vclk_interrupt callback */
void MSM5205I_data_w (int num, int data);
/* VCLK slave mode option                                        */
/* if VCLK and reset or data is changed at the same time,        */
/* Call MSM5205I_vclk_w after MSM5205I_data_w and MSM5205I_reset_w. */
void MSM5205I_vclk_w (int num, int reset);
/* option , selected pin seletor */
void MSM5205I_selector_w (int num, int _select);
void MSM5205I_bitwidth_w (int num, int bitwidth);
void MSM5205I_set_volume(int num,int volume);
static void MSM5205I_set_timer(int num,int select);

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif
#endif
