#ifndef _CPS1_H_
#define _CPS1_H_

#if defined(__cplusplus) && !defined(USE_CPLUS)
extern "C" {
#endif

extern unsigned char * cps1_gfxram;     /* Video RAM */
extern unsigned char * cps1_output;     /* Output ports */
extern int cps1_gfxram_size;
extern int cps1_output_size;

int cps1_output_r(int offset);
void cps1_output_w(int offset, int data);
int cps1_gfxram_r(int offset);
void cps1_gfxram_w(int offset, int data);

int  cps1_vh_start(void);
void cps1_vh_stop(void);
void cps1_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);

int cps1_eeprom_port_r(int offset);
void cps1_eeprom_port_w(int offset, int data);

/* Game specific data */
struct CPS1config
{
        char *name;             /* game driver name */
        int   bank_obj;
        int   bank_scroll1;
        int   bank_scroll2;
        int   bank_scroll3;
        int   alternative;      /* Game alternative */
        int   cpsb_addr;        /* CPS board B test register address */
        int   cpsb_value;       /* CPS board B test register expected value */
        int   gng_sprite_kludge;  /* Ghouls n Ghosts sprite kludge */
};

extern struct CPS1config *cps1_game_config;

#if defined(__cplusplus) && !defined(USE_CPLUS)
}
#endif

#endif
