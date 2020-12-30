/***************************************************************************
 *   Twin Cobra							    *
 *   Communications and memory functions between shared CPU memory spaces  *
 ***************************************************************************/

#include "driver.h"
#include "tms32010/tms32010.h"


unsigned char *twincobr_68k_dsp_ram;
unsigned char *twincobr_7800c;
unsigned char *twincobr_sharedram;
unsigned char *wardner_mainram;

extern unsigned char *spriteram;
extern unsigned char *paletteram;

extern int twincobr_fg_rom_bank;
extern int twincobr_bg_ram_bank;
extern int twincobr_display_on;
extern int twincobr_flip_screen;
extern int wardner_sprite_hack;

static int dsp_exec = 0;
static unsigned int dsp_addr_w = 0, main_ram_seg;
static int toaplan_main_cpu = 0;   /* Main CPU type.  0 = 68000, 1 = Z80 */
static int dsp_accessed = 0;
int intenable = 0;



int twincobr_dsp_in(int offset)
{
	if (toaplan_main_cpu == 0)
	{
		unsigned int input_data = 0;
		input_data = READ_WORD(&twincobr_68k_dsp_ram[dsp_addr_w]);
		dsp_accessed = 1;
		return input_data;
	}

	/* DSP can read data from 68000 main RAM via DSP IO port 1 */

        unsigned int input_data = 0;
        switch (main_ram_seg) {
                case 0x30000:   input_data = READ_WORD(&twincobr_68k_dsp_ram[dsp_addr_w]); dsp_accessed = 1; break;
                case 0x40000:   input_data = READ_WORD(&spriteram[dsp_addr_w]); break;
                case 0x50000:   input_data = READ_WORD(&paletteram[dsp_addr_w]); break;
                case 0x7000:    input_data = wardner_mainram[dsp_addr_w] + (wardner_mainram[dsp_addr_w+1]<<8); break;
                case 0x8000:    input_data = spriteram[dsp_addr_w] + (spriteram[dsp_addr_w+1]<<8); break;
                case 0xa000:    input_data = paletteram[dsp_addr_w] + (paletteram[dsp_addr_w+1]<<8); break;
        }
        return input_data;

}

void twincobr_dsp_out(int fnction,int data)
{
	if (toaplan_main_cpu == 0)
	{
		if (fnction == 0) {
		    dsp_addr_w = ((data & 0x01fff) << 1);
		    dsp_accessed = 0;
		}
		if (fnction == 1) {
		    if ((dsp_accessed) && (data == 0)) {
			 dsp_addr_w = 0;
			 dsp_exec = 1;
		    }
		    else dsp_exec = 0;
		    WRITE_WORD(&twincobr_68k_dsp_ram[dsp_addr_w],data);
		    dsp_accessed = 1;
		}
		if (fnction == 3) {
		    if (data & 0x8000) {
			cpu_cause_interrupt(2,TMS320C10_IGNORE_BIO);
		    }
		    if (data == 0) {
			if (dsp_exec) {
			    cpu_halt(0,1);
			    dsp_exec = 0;
			}
			  cpu_cause_interrupt(2,TMS320C10_ACTIVE_BIO);
		    }
		}
		return;
	}

	if (fnction == 0) {
	    /* This sets the 68000 main RAM address the DSP should */
	    /*    read/write, via the DSP IO port 0 */
	    /* Top three bits of data need to be shifted left 3 places */
	    /*    to select which 64K memory bank from 68000 space to use */
	    /*    Here though, it only uses memory bank 0x03xxxx, so they */
	    /*    are masked for simplicity */
	    /* All of this data is shifted left one position to move it */
	    /*    to the 68000 word boundary equivalent */

		dsp_addr_w = ((data & 0x01fff) << 1);
		main_ram_seg = ((data & 0xe000) << 3);
		if (toaplan_main_cpu == 1) {            /* Z80 */
			 dsp_addr_w &= 0xfff;
			 if (main_ram_seg == 0x30000) main_ram_seg = 0x7000;
			 if (main_ram_seg == 0x40000) main_ram_seg = 0x8000;
			 if (main_ram_seg == 0x50000) main_ram_seg = 0xa000;
		}
	}
	if (fnction == 1)
	{

		/* Data written to main CPU RAM via DSP IO port 1*/
		dsp_exec = 0;

		switch (main_ram_seg) {
			case 0x30000:   WRITE_WORD(&twincobr_68k_dsp_ram[dsp_addr_w],data);
					if ((dsp_addr_w < 3) && (data == 0)) dsp_exec = 1; break;
			case 0x40000:   WRITE_WORD(&spriteram[dsp_addr_w],data); break;
			case 0x50000:   WRITE_WORD(&paletteram[dsp_addr_w],data); break;
			case 0x7000:    wardner_mainram[dsp_addr_w] = data & 0xff;
					wardner_mainram[dsp_addr_w + 1] = (data >> 8) & 0xff;
					if ((dsp_addr_w < 3) && (data == 0)) dsp_exec = 1; break;
			case 0x8000:    spriteram[dsp_addr_w] = data & 0xff;
					spriteram[dsp_addr_w + 1] = (data >> 8) & 0xff;break;
			case 0xa000:    paletteram[dsp_addr_w] = data & 0xff;
					paletteram[dsp_addr_w + 1] = (data >> 8) & 0xff; break;
		}
	}
	if (fnction == 3) {
	    /* data 0xffff means inhibit BIO line to DSP and enable  */
	    /*	     communication to 68000 processor */
	    /*	     Actually only DSP data bit 15 controls this */
	    /* data 0x0000 means set DSP BIO line active and disable */
	    /*	     communication to 68000 processor*/
	    if (data & 0x8000) {
		cpu_cause_interrupt(2,TMS320C10_IGNORE_BIO);
	    }
	    if (data == 0)
	    {
		if (dsp_exec)
		{
			cpu_halt(0,1);
			dsp_exec = 0;
		}
		cpu_cause_interrupt(2,TMS320C10_ACTIVE_BIO);
	    }
	}
}

int twincobr_68k_dsp_r(int offset)
{
	return READ_WORD(&twincobr_68k_dsp_ram[offset]);
}

void twincobr_68k_dsp_w(int offset,int data)
{
	COMBINE_WORD_MEM(&twincobr_68k_dsp_ram[offset],data);
}

void wardner_mainram_w(int offset, int data)
{
        wardner_mainram[offset] = data;
}

int wardner_mainram_r(int offset)
{
        return wardner_mainram[offset];
}



int twincobr_7800c_r(int offset)
{
	return READ_WORD(&twincobr_7800c[offset]);
}

void twincobr_7800c_w(int offset,int data)
{
#if 0
	if (toaplan_main_cpu == 1) {
		if (data == 0x0c) { data = 0x1c; wardner_sprite_hack=0; }
		if (data == 0x0d) { data = 0x1d; wardner_sprite_hack=1; }
	}
#endif
	if (toaplan_main_cpu == 0)
	{
		WRITE_WORD(&twincobr_7800c[offset],data);
	}

	switch (data) {
	    case 0x0004: intenable = 0; break;
	    case 0x0005: intenable = 1; break;
	    case 0x0006: twincobr_flip_screen = 0; break;
	    case 0x0007: twincobr_flip_screen = 1; break;
	    case 0x0008: twincobr_bg_ram_bank = 0x0000; break;
	    case 0x0009: twincobr_bg_ram_bank = 0x2000; break;
	    case 0x000a: twincobr_fg_rom_bank = 0x0000; break;
	    case 0x000b: twincobr_fg_rom_bank = 0x1000; break;
	    case 0x000e: twincobr_display_on  = 0x0000; break; /* Turn display off */
	    case 0x000f: twincobr_display_on  = 0x0001; break; /* Turn display on */
	    case 0x000c: if (twincobr_display_on) {
			   /* This means assert the INT line to the DSP */
			     cpu_halt(2,1);
			     cpu_cause_interrupt(2,TMS320C10_ACTIVE_INT);
			     cpu_halt(0,0);
			 }
			 break;
	    case 0x000d: if (twincobr_display_on) {
			   /* This means inhibit the INT line to the DSP */
			     cpu_clear_pending_interrupts(2);
			     cpu_halt(2,0);
			 }
			 break;
	}
}


int twincobr_sharedram_r(int offset)
{
	return twincobr_sharedram[offset / 2];
}

void twincobr_sharedram_w(int offset,int data)
{
	twincobr_sharedram[offset / 2] = data;
}

void twincobr_reset(void)
{
        toaplan_main_cpu = 0;           /* M68000 */
        intenable = 0;
        twincobr_display_on = 1;
        dsp_addr_w = dsp_exec = 0;
}



void wardner_reset(void)
{
        /* clean out high score tables in these game hardware */
        wardner_mainram[0x0117] = 0xff;
        wardner_mainram[0x0118] = 0xff;
        wardner_mainram[0x0119] = 0xff;
        wardner_mainram[0x011a] = 0xff;
        wardner_mainram[0x011b] = 0xff;
        wardner_mainram[0x0170] = 0xff;
        wardner_mainram[0x0171] = 0xff;
        wardner_mainram[0x0172] = 0xff;

        toaplan_main_cpu = 1;           /* Z80 */
        intenable = 0;
        twincobr_display_on = 1;
        dsp_addr_w = dsp_exec = 0;
        main_ram_seg = 0;

        /* blank out the screen */
        osd_clearbitmap(Machine->scrbitmap);
}

void fshark_coin_dsp_w(int offset,int data)
{
	 switch (data) {
                case 0x00:      /* This means assert the INT line to the DSP */
			cpu_halt(2,1);
			cpu_cause_interrupt(2,TMS320C10_ACTIVE_INT);
			cpu_halt(0,0);
			break;
		case 0x01:      /* This means inhibit the INT line to the DSP */
			cpu_clear_pending_interrupts(2);
			cpu_halt(2,0);
			break;
        }
}
