/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/


#include "driver.h"
#include <math.h>


#define  KDAC_A_PCM_MAX    (2)


typedef struct kdacApcm
{
	unsigned char vol[KDAC_A_PCM_MAX];
	unsigned int  addr[KDAC_A_PCM_MAX];
	unsigned int  start[KDAC_A_PCM_MAX];
	unsigned int  step[KDAC_A_PCM_MAX];
	int play[KDAC_A_PCM_MAX];
	int loop[KDAC_A_PCM_MAX];

	unsigned char wreg[0x10]; /* write data */
} KDAC_A_PCM;

static KDAC_A_PCM    kpcm;

static int pcm_chan;

static unsigned char *pcmbuf[2];
static unsigned int  pcm_limit;


#define   BASE_SHIFT    (12)



int kdac_note[] = {
  261.63/8, 277.18/8,
  293.67/8, 311.13/8,
  329.63/8,
  349.23/8, 369.99/8,
  392.00/8, 415.31/8,
  440.00/8, 466.16/8,
  493.88/8,

  523.25/8,
};

static float fncode[0x200];
/*************************************************************/
void KDAC_A_make_fncode( void ){
  int i;
  for( i = 0; i < 0x200; i++ ){
    fncode[i] = (0x200 * 55) / (0x200 - i);
  }
}


/************************************************/
/*    Konami PCM update                         */
/************************************************/

void KDAC_A_update(int arg, void **buffer, int buffer_len)
{
	int i;
	int sample_bits;
	signed char *buf8[2];
	signed short *buf16[2];

	sample_bits = arg >> 8;

	buf8[0] = (signed char *)buffer[0];
	buf8[1] = (signed char *)buffer[1];
	buf16[0] = (signed short *)buffer[0];
	buf16[1] = (signed short *)buffer[1];

	for( i = 0; i < KDAC_A_PCM_MAX; i++ )
	{
		if (kpcm.play[i])
		{
			int vol,j;
			unsigned int addr, old_addr;

			/**** PCM setup ****/
			addr = kpcm.start[i] + ((kpcm.addr[i]>>BASE_SHIFT)&0x000fffff);
			vol = kpcm.vol[i] * 0x22;
			for( j = 0; j < buffer_len; j++ )
			{
				old_addr = addr;
				addr = kpcm.start[i] + ((kpcm.addr[i]>>BASE_SHIFT)&0x000fffff);
				while (old_addr <= addr)
				{
					if (pcmbuf[i][old_addr] & 0x80)
					{
						/* end of sample */

						if (kpcm.loop[i])
						{
							/* loop to the beginning */
							addr = kpcm.start[i];
							kpcm.addr[i] = 0;
						}
						else
						{
							/* stop sample */
							kpcm.play[i] = 0;
						}
						break;
					}

					old_addr++;
				}

				if (kpcm.play[i] == 0)
				{
					if (sample_bits == 8)
						fast_memset(buf8[i],0,(buffer_len - j) * sizeof(signed char));
					else
						fast_memset(buf16[i],0,(buffer_len - j) * sizeof(signed short));
					break;
				}

				kpcm.addr[i] += kpcm.step[i];

				if (sample_bits == 8)
					buf8[i][j] = (((pcmbuf[i][addr] & 0x7f) - 0x40) * vol) >> 8;
				else
					buf16[i][j] = (((pcmbuf[i][addr] & 0x7f) - 0x40) * vol);
			}
		}
		else
		{
			if (sample_bits == 8)
				fast_memset(buf8[i],0,buffer_len * sizeof(signed char));
			else
				fast_memset(buf16[i],0,buffer_len * sizeof(signed short));
		}
	}
}

/************************************************/
/*    Konami PCM start                          */
/************************************************/
int K007232_sh_start( struct K007232_interface *intf )
{
	int i;
	char buf[2][40];
	const char *name[2];

	pcmbuf[0] = (unsigned char *)Machine->memory_region[intf->bankA];
	pcmbuf[1] = (unsigned char *)Machine->memory_region[intf->bankB];
	if( !intf->limit )    pcm_limit = 0x00020000;	/* default limit */
	else                  pcm_limit = intf->limit;

	for( i = 0; i < KDAC_A_PCM_MAX; i++ )
	{
		kpcm.vol[i] = 0;
		kpcm.start[i] = 0;
		kpcm.step[i] = 0;
		kpcm.play[i] = 0;
		kpcm.loop[i] = 0;
	}
	for( i = 0; i < 0x10; i++ )  kpcm.wreg[i] = 0;

	for (i = 0;i < 2;i++)
	{
		name[i] = buf[i];
		sprintf(buf[i],"Konami 007232 Ch %c",'A'+i);
	}
	pcm_chan = stream_init_multi(2,name,Machine->sample_rate,
			Machine->sample_bits,Machine->sample_bits << 8,KDAC_A_update);
	stream_set_volume(pcm_chan,intf->volume);
	stream_set_volume(pcm_chan+1,intf->volume);
	KDAC_A_make_fncode();

	return 0;
}

/************************************************/
/*    Konami PCM stop                           */
/************************************************/
void K007232_sh_stop( void )
{
}

void K007232_sh_update( void )
{
}

/************************************************/
/*    Konami PCM write register                 */
/************************************************/
void K007232_WriteReg( int r, int v )
{
	int  data;

	if (Machine->sample_rate == 0) return;

	stream_update(pcm_chan,0);

	kpcm.wreg[r] = v;			/* stock write data */

	if (r == 0x0d)
	{
		/* select if sample plays once or looped */
		kpcm.loop[0] = v & 0x01;
		kpcm.loop[1] = v & 0x02;
		return;
	}
	else if (r == 0x0c)
	{
		/* volume */
		/* colume is externally controlled so it could be anything, but I guess */
		/* this is a standard setup. It's the one used in TMNT. */
		/* the pin is marked BLEV so it could be intended to control only channel B */

		kpcm.vol[0] = (v >> 4) & 0x0f;
		kpcm.vol[1] = (v >> 0) & 0x0f;
		return;
	}
	else
	{
		int  reg_port;

		reg_port = 0;
		if (r >= 0x06)
		{
			reg_port = 1;
			r -= 0x06;
		}

		switch (r)
		{
			case 0x00:
			case 0x01:
				/**** address step ****/
				data = (((((unsigned int)kpcm.wreg[reg_port*0x06 + 0x01])<<8)&0x0100) | (((unsigned int)kpcm.wreg[reg_port*0x06 + 0x00])&0x00ff));

				kpcm.step[reg_port] =
					( (7850.0 / (float)Machine->sample_rate) ) *
					( fncode[data] / (440.00/2) ) *
					( (float)3580000 / (float)4000000 ) *
					(1<<BASE_SHIFT);
				break;

			case 0x02:
			case 0x03:
			case 0x04:
				/**** start address ****/
				kpcm.start[reg_port] =
					((((unsigned int)kpcm.wreg[reg_port*0x06 + 0x04]<<16)&0x00010000) |
					(((unsigned int)kpcm.wreg[reg_port*0x06 + 0x03]<< 8)&0x0000ff00) |
					(((unsigned int)kpcm.wreg[reg_port*0x06 + 0x02]    )&0x000000ff));
			break;
		}
	}
}

/************************************************/
/*    Konami PCM read register                  */
/************************************************/
void check_err_fncode( int data ){
  /**** not make keycode ****/
}

int  K007232_ReadReg( int r )
{
	if (r == 0x0005)
	{
		if (kpcm.start[0] < pcm_limit)
		{
			kpcm.play[0] = 1;
			kpcm.addr[0] = 0;
		}
	}
	else if (r == 0x000b)
	{
		if (kpcm.start[1] < pcm_limit)
		{
			kpcm.play[1] = 1;
			kpcm.addr[1] = 0;
		}
	}
	return 0;
}
