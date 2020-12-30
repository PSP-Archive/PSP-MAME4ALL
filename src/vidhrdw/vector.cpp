/******************************************************************************
 *
 * vector.c
 *
 *
 * Copyright 1997,1998 by the M.A.M.E. Project
 *
 *        anti-alias code by Andrew Caldwell
 *        (still more to add)
 *
 * 980611 use translucent vectors. Thanks to Peter Hirschberg
 *        and Neil Bradley for the inspiration. BW
 * 980307 added cleverer dirty handling. BW, ASG
 *        fixed antialias table .ac
 * 980221 rewrote anti-alias line draw routine
 *        added inline assembly multiply fuction for 8086 based machines
 *        beam diameter added to draw routine
 *        beam diameter is accurate in anti-alias line draw (Tcosin)
 *        flicker added .ac
 * 980203 moved LBO's routines for drawing into a buffer of vertices
 *        from avgdvg.c to this location. Scaling is now initialized
 *        by calling vector_init(...). BW
 * 980202 moved out of msdos.c ASG
 * 980124 added anti-alias line draw routine
 *        modified avgdvg.c and sega.c to support new line draw routine
 *        added two new tables Tinten and Tmerge (for 256 color support)
 *        added find_color routine to build above tables .ac
 *
 **************************************************************************** */

/* GLmame and FXmame provide their own vector implementations */
#if !(defined xgl) && !(defined xfx) && !(defined svgafx)

#include <math.h>
#include "osinline.h"
#include "driver.h"
#include "osdepend.h"
#include "vector.h"

#define VCLEAN  0
#define VDIRTY  1
#define VCLIP   2

unsigned char *vectorram;
int vectorram_size;

int antialias;                            /* flag for anti-aliasing */
int beam;                                 /* size of vector beam    */
int flicker;                              /* beam flicker value     */
int translucency;

static int beam_diameter_is_one = 0;      /* flag that beam is one pixel wide */

static int vector_scale_x;                /* scaling to screen */
static int vector_scale_y;                /* scaling to screen */

/* The vectices are buffered here */
typedef struct
{
	int x; int y;
	int col;
	int intensity;
	int arg1; int arg2; /* start/end in pixel array or clipping info */
	int status;         /* for dirty and clipping handling */
} point;

static point *new_list;
static point *old_list;
static int new_index;
static int old_index;

/* coordinates of pixels are stored here for faster removal */
static unsigned int *pixel;
static int p_index=0;

static unsigned int  *pTcosin;            /* adjust line width */
static unsigned char *pTinten;            /* intensity         */
static unsigned char *pTmerge;            /* mergeing pixels   */

#define Tcosin(x)   pTcosin[(x)]          /* adjust line width */
#define Tinten(x,y) pTinten[(x)*256+(y)]  /* intensity         */
#define Tmerge(x,y) pTmerge[(x)*256+(y)]  /* mergeing pixels   */

#define ANTIALIAS_GUNBIT  6             /* 6 bits per gun in vga (1-8 valid) */
#define ANTIALIAS_GUNNUM  (1<<ANTIALIAS_GUNBIT)

static unsigned char Tgamma[256];         /* quick gamma anti-alias table  */
static unsigned char Tgammar[256];        /* same as above, reversed order */

static struct osd_bitmap *vecbitmap;
static int vecwidth, vecheight;
static int vecshift;
static int xmin, ymin, xmax, ymax; /* clipping area */

static int vector_runs;	/* vector runs per refresh */

/*
 * multiply and divide routines for drawing lines
 * can be be replaced by an assembly routine in osinline.h
 */
#ifndef vec_mult
INLINE int vec_mult(int parm1, int parm2)
{
	int temp,result;

	temp     = abs(parm1);
	result   = (temp&0x0000ffff) * (parm2&0x0000ffff);
	result >>= 16;
	result  += (temp&0x0000ffff) * (parm2>>16       );
	result  += (temp>>16       ) * (parm2&0x0000ffff);
	result >>= 16;
	result  += (temp>>16       ) * (parm2>>16       );

	if( parm1 < 0 )
		return(-result);
	else
		return( result);
}
#endif

/* can be be replaced by an assembly routine in osinline.h */
#ifndef vec_div
INLINE int vec_div(int parm1, int parm2)
{
	if( (parm2>>12) )
	{
		parm1 = (parm1<<4) / (parm2>>12);
		if( parm1 > 0x00010000 )
			return( 0x00010000 );
		if( parm1 < -0x00010000 )
			return( -0x00010000 );
		return( parm1 );
	}
	return( 0x00010000 );
}
#endif


/*
 * finds closest color and returns the index (for 256 color)
 */

static unsigned char find_color(unsigned char r,unsigned char g,unsigned char b)
{
	int i,bi,ii;
	long x,y,z,bc;
	ii = 32;
	bi = 256;
	bc = 0x01000000;

	do
	{
		for( i=0; i<256; i++ )
		{
			unsigned char r1,g1,b1;

			osd_get_pen(i,&r1,&g1,&b1);
			if((x=(long)(abs(r1-r)+1)) > ii) continue;
			if((y=(long)(abs(g1-g)+1)) > ii) continue;
			if((z=(long)(abs(b1-b)+1)) > ii) continue;
			x = x*y*z;
			if (x < bc)
			{
				bc = x;
				bi = i;
			}
		}
		ii<<=1;
	} while (bi==256);

	return(bi);
}


/*
 * Initializes vector game video emulation
 */

int vector_vh_start (void)
{
	int h,i,j,k,c[3];

	if (beam == 0x00010000)
		beam_diameter_is_one = 1;

	p_index=0;

	new_index = 0;
	old_index = 0;
	vector_runs = 0;

	/* allocate memory for tables */
	pTcosin = (unsigned *)gp2x_malloc ( (2048+1) * sizeof(int));   /* yes! 2049 is correct */
	pTinten = (unsigned char *)gp2x_malloc ( 256 * 256 * sizeof(unsigned char));
	pTmerge = (unsigned char *)gp2x_malloc (256 * 256 * sizeof(unsigned char));
	pixel = (unsigned *)gp2x_malloc (MAX_PIXELS * sizeof (unsigned int));
	old_list = (point *)gp2x_malloc (MAX_POINTS * sizeof (point));
	new_list = (point *)gp2x_malloc (MAX_POINTS * sizeof (point));

	/* did we get the requested memory? */
	if (!(pTcosin && pTinten && pTmerge && pixel && old_list && new_list))
	{
		/* vector_vh_stop should better be called by the main engine */
		/* if vector_vh_start fails */
		vector_vh_stop();
		return 1;
	}

	/* build cosine table for fixing line width in antialias */
	for (i=0; i<=2048; i++)
	{
		Tcosin(i) = (int)((double)(1.0/cos(atan((double)(i)/2048.0)))*0x10000000 + 0.5);
		/*printf ("cos %08x\n", Tcosin(i) );*/
	}

	/* build anti-alias table */
	h = 256 / ANTIALIAS_GUNNUM;           /* to generate table faster */
	for (i=0; i<256; i+=h )               /* intensity */
	{
		for (j=0; j<256; j++)               /* color */
		{
			unsigned char r1,g1,b1,col,n;
			osd_get_pen(j,&r1,&g1,&b1);
			col = find_color( (r1*(i+1))>>8, (g1*(i+1))>>8, (b1*(i+1))>>8 );
			for (n=0; n<h; n++ )
			{
				Tinten(i+n,j) = col;
			}
		}
	}

	/* build merge color table */
	for( i=0; i<256 ;i++ )                /* color1 */
	{
		unsigned char rgb1[3],rgb2[3];
		osd_get_pen(i,&rgb1[0],&rgb1[1],&rgb1[2]);
		for( j=0; j<=i ;j++ )               /* color2 */
		{
			osd_get_pen(j,&rgb2[0],&rgb2[1],&rgb2[2]);

			for (k=0; k<3; k++)
			if (translucency) /* add gun values */
			{
				int tmp;
				tmp = rgb1[k] + rgb2[k];
				if (tmp > 255)
					c[k] = 255;
				else
					c[k] = tmp;
			}
			else /* choose highest gun value */
			{
				if (rgb1[k] > rgb2[k])
					c[k] = rgb1[k];
				else
					c[k] = rgb2[k];
			}
			Tmerge(i,j) = Tmerge(j,i) = find_color(c[0],c[1],c[2]);
		}
	}

	/* build gamma correction table */
	for (i=0; i<256; i++)
	{
		h = i + (i>>2);
		if( h > 255) h = 255;
		Tgamma[i] = Tgammar[255-i] = h;
	}

	return 0;
}


/*
 * Setup scaling. Currently the Sega games are stuck at VECSHIFT 15
 * and the the AVG games at VECSHIFT 16
 */
void vector_set_shift (int shift)
{
	vecshift = shift;
}

/*
 * Clear the old bitmap. Delete pixel for pixel, this is faster than fast_memset.
 */
static void vector_clear_pixels (void)
{
	unsigned char bg=Machine->pens[0];
	int i;
	int coords;


	for (i=p_index-1; i>=0; i--)
	{
		coords = pixel[i];
		vecbitmap->line[coords & 0x0000ffff][coords >> 16] = bg;
	}

	p_index=0;

}

/*
 * Stop the vector video hardware emulation. Free memory.
 */
void vector_vh_stop (void)
{
	if (pTcosin)
		gp2x_free (pTcosin);
	pTcosin = NULL;
	if (pTinten)
		gp2x_free (pTinten);
	pTinten = NULL;
	if (pTmerge)
		gp2x_free (pTmerge);
	pTmerge = NULL;
	if (pixel)
		gp2x_free (pixel);
	pixel = NULL;
	if (old_list)
		gp2x_free (old_list);
	old_list = NULL;
	if (new_list)
		gp2x_free (new_list);
	new_list = NULL;
}

/*
 * draws an anti-aliased pixel (blends pixel with background)
 */
INLINE void vector_draw_aa_pixel (int x, int y, int col, int dirty)
{
	unsigned char *address;

	if (x < xmin || x >= xmax)
		return;
	if (y < ymin || y >= ymax)
		return;

	address=&(vecbitmap->line[y][x]);
	*address=(unsigned char)Tmerge(*address,(unsigned char)(col));

	if (p_index<MAX_PIXELS)
	{
		pixel[p_index]=y | (x << 16);
		p_index++;
	}
}


/*
 * draws a line
 *
 * input:   x2  16.16 fixed point
 *          y2  16.16 fixed point
 *         col  0-255 indexed color (8 bit)
 *   intensity  0-255 intensity
 *       dirty  bool  mark the pixels as dirty while plotting them
 *
 * written by Andrew Caldwell
 */

void vector_draw_to (int x2, int y2, int col, int intensity, int dirty)
{
	unsigned char a1;
	int orientation;
	int dx,dy,sx,sy,cx,cy,width;
	static int x1,yy1;
	int xx,yy;

	/* [1] scale coordinates to display */

	x2 = vec_mult(x2<<4,vector_scale_x);
	y2 = vec_mult(y2<<4,vector_scale_y);

	/* [2] fix display orientation */

	orientation = Machine->orientation;
	if (orientation != ORIENTATION_DEFAULT)
	{
		if (orientation & ORIENTATION_SWAP_XY)
		{
			int temp;
			temp = x2;
			x2 = y2;
			y2 = temp;
		}
		if (orientation & ORIENTATION_FLIP_X)
			x2 = ((vecwidth-1)<<16)-x2;
		if (orientation & ORIENTATION_FLIP_Y)
			y2 = ((vecheight-1)<<16)-y2;
	}

	/* [3] adjust cords if needed */

	if (antialias)
	{
		if(beam_diameter_is_one)
		{
			x2 = (x2+0x8000)&0xffff0000;
			y2 = (y2+0x8000)&0xffff0000;
		}
	}
	else /* noantialiasing */
	{
		x2 >>= 16;
		y2 >>= 16;
	}

	/* [4] handle color and intensity */

	if (intensity == 0) goto end_draw;

	col = Tinten(intensity,Machine->pens[col]);

	/* [5] draw line */

	if (antialias)
	{
		/* draw an anti-aliased line */
		dx=abs(x1-x2);
		dy=abs(yy1-y2);
		if (dx>=dy)
		{
			sx = ((x1 <= x2) ? 1:-1);
			sy = vec_div(y2-yy1,dx);
			if (sy<0)
				dy--;
			x1 >>= 16;
			xx = x2>>16;
			width = vec_mult(beam<<4,Tcosin(abs(sy)>>5));
			if (!beam_diameter_is_one)
				yy1-= width>>1; /* start back half the diameter */
			for (;;)
			{
				dx = width;    /* init diameter of beam */
				dy = yy1>>16;
				vector_draw_aa_pixel(x1,dy++,Tinten(Tgammar[0xff&(yy1>>8)],col), dirty);
				dx -= 0x10000-(0xffff & yy1); /* take off amount plotted */
				a1 = Tgamma[(dx>>8)&0xff];   /* calc remainder pixel */
				dx >>= 16;                   /* adjust to pixel (solid) count */
				while (dx--)                 /* plot rest of pixels */
					vector_draw_aa_pixel(x1,dy++,col, dirty);
				vector_draw_aa_pixel(x1,dy,Tinten(a1,col), dirty);
				if (x1 == xx) break;
				x1+=sx;
				yy1+=sy;
			}
		}
		else
		{
			sy = ((yy1 <= y2) ? 1:-1);
			sx = vec_div(x2-x1,dy);
			if (sx<0)
				dx--;
			yy1 >>= 16;
			yy = y2>>16;
			width = vec_mult(beam<<4,Tcosin(abs(sx)>>5));
			if( !beam_diameter_is_one )
				x1-= width>>1; /* start back half the width */
			for (;;)
			{
				dy = width;    /* calc diameter of beam */
				dx = x1>>16;
				vector_draw_aa_pixel(dx++,yy1,Tinten(Tgammar[0xff&(x1>>8)],col), dirty);
				dy -= 0x10000-(0xffff & x1); /* take off amount plotted */
				a1 = Tgamma[(dy>>8)&0xff];   /* remainder pixel */
				dy >>= 16;                   /* adjust to pixel (solid) count */
				while (dy--)                 /* plot rest of pixels */
					vector_draw_aa_pixel(dx++,yy1,col, dirty);
				vector_draw_aa_pixel(dx,yy1,Tinten(a1,col), dirty);
				if (yy1 == yy) break;
				yy1+=sy;
				x1+=sx;
			}
		}
	}
	else /* use good old Bresenham for non-antialiasing 980317 BW */
	{
		dx = abs(x1-x2);
		dy = abs(yy1-y2);
		sx = (x1 <= x2) ? 1: -1;
		sy = (yy1 <= y2) ? 1: -1;
		cx = dx/2;
		cy = dy/2;

		if (dx>=dy)
		{
			for (;;)
			{
				vector_draw_aa_pixel (x1, yy1, col, dirty);
				if (x1 == x2) break;
				x1 += sx;
				cx -= dy;
				if (cx < 0)
				{
					yy1 += sy;
					cx += dx;
				}
			}
		}
		else
		{
			for (;;)
			{
				vector_draw_aa_pixel (x1, yy1, col, dirty);
				if (yy1 == y2) break;
				yy1 += sy;
				cy -= dx;
				if (cy < 0)
				{
					x1 += sx;
					cy += dy;
				}
			}
		}
	}

end_draw:

	x1=x2;
	yy1=y2;
}


/*
 * Adds a line end point to the vertices list. The vector processor emulation
 * needs to call this.
 */
void vector_add_point (int x, int y, int color, int intensity)
{
	point *thenew;

	if (flicker && (intensity > 0))
	{
		intensity += (intensity * (0x80-(rand()&0xff)) * flicker)>>16;
		if (intensity < 0)
			intensity = 0;
		if (intensity > 0xff)
			intensity = 0xff;
	}
	thenew = &new_list[new_index];
	thenew->x = x;
	thenew->y = y;
	thenew->col = color;
	thenew->intensity = intensity;
	thenew->status = VDIRTY; /* mark identical lines as clean later */

	new_index++;
	if (new_index >= MAX_POINTS)
	{
		new_index--;
	}
}

/*
 * Add new clipping info to the list
 */
void vector_add_clip (int x1, int yy1, int x2, int y2)
{
	point *thenew;

	thenew = &new_list[new_index];
	thenew->x = x1;
	thenew->y = yy1;
	thenew->arg1 = x2;
	thenew->arg2 = y2;
	thenew->status = VCLIP;

	new_index++;
	if (new_index >= MAX_POINTS)
	{
		new_index--;
	}
}


/*
 * Set the clipping area
 */
void vector_set_clip (int x1, int yy1, int x2, int y2)
{
	int orientation;
	int tmp;

	/* failsafe */
	if ((x1 >= x2) || (yy1 >= y2))
	{
		xmin = 0;
		ymin = 0;
		xmax = vecwidth;
		ymax = vecheight;
		return;
	}

	/* scale coordinates to display */
	x1 = vec_mult(x1<<4,vector_scale_x);
	yy1 = vec_mult(yy1<<4,vector_scale_y);
	x2 = vec_mult(x2<<4,vector_scale_x);
	y2 = vec_mult(y2<<4,vector_scale_y);

	/* fix orientation */
	orientation = Machine->orientation;
	if (orientation != ORIENTATION_DEFAULT)
	{
		/* swapping x/y coordinates will still have the minima in x1,yy1 */
		if (orientation & ORIENTATION_SWAP_XY)
		{
			tmp = x1; x1 = yy1; yy1 = tmp;
			tmp = x2; x2 = y2; y2 = tmp;
		}
		/* don't forget to swap x1,x2, since x2 becomes the minimum */
		if (orientation & ORIENTATION_FLIP_X)
		{
			x1 = ((vecwidth-1)<<16)-x1;
			x2 = ((vecwidth-1)<<16)-x2;
			tmp = x1; x1 = x2; x2 = tmp;
		}
		/* don't forget to swap yy1,y2, since y2 becomes the minimum */
		if (orientation & ORIENTATION_FLIP_Y)
		{
			yy1 = ((vecheight-1)<<16)-yy1;
			y2 = ((vecheight-1)<<16)-y2;
			tmp = yy1; yy1 = y2; y2 = tmp;
		}
	}

	xmin = x1 >> 16;
	ymin = yy1 >> 16;
	xmax = x2 >> 16;
	ymax = y2 >> 16;

	/* Make it foolproof by trapping rounding errors */
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > vecwidth) xmax = vecwidth;
	if (ymax > vecheight) ymax = vecheight;
}


/*
 * The vector CPU creates a new display list. We save the old display list,
 * but only once per refresh.
 */
void vector_clear_list (void)
{
	point *tmp;

	if (vector_runs == 0)
	{
		old_index = new_index;
		tmp = old_list; old_list = new_list; new_list = tmp;
	}

	new_index = 0;
	vector_runs++;
}


/*
 * By comparing with the last drawn list, we can prevent that identical
 * vectors are marked dirty which appeared at the same list index in the
 * previous frame. BW 19980307
 */
static void clever_mark_dirty (void)
{
	int i, j, min_index, last_match = 0;
	unsigned int *coords;
	point *thenew, *old;
	point newclip, oldclip;
	int clips_match = 1;

	if (old_index < new_index)
		min_index = old_index;
	else
		min_index = new_index;

	/* Reset the active clips to invalid values */
	fast_memset (&newclip, 0, sizeof (newclip));
	fast_memset (&oldclip, 0, sizeof (oldclip));

	/* Mark vectors which are not the same in both lists as dirty */
	thenew = new_list;
	old = old_list;

	for (i = min_index; i > 0; i--, old++, thenew++)
	{
		/* If this is a clip, we need to determine if the clip regions still match */
		if (old->status == VCLIP || thenew->status == VCLIP)
		{
			if (old->status == VCLIP)
				oldclip = *old;
			if (thenew->status == VCLIP)
				newclip = *thenew;
			clips_match = (newclip.x == oldclip.x) && (newclip.y == oldclip.y) && (newclip.arg1 == oldclip.arg1) && (newclip.arg2 == oldclip.arg2);
			if (!clips_match)
				last_match = 0;

			/* fall through to erase the old line if this is not a clip */
			if (old->status == VCLIP)
				continue;
		}

		/* If the clips match and the vectors match, update */
		else if (clips_match && (thenew->x == old->x) && (thenew->y == old->y) &&
			(thenew->col == old->col) && (thenew->intensity == old->intensity))
		{
			if (last_match)
			{
				thenew->status = VCLEAN;
				continue;
			}
			last_match = 1;
		}

		/* If nothing matches, remember it */
		else
			last_match = 0;

		/* mark the pixels of the old vector dirty */
		coords = &pixel[old->arg1];
		for (j = (old->arg2 - old->arg1); j > 0; j--)
		{
			coords++;
		}
	}

	/* all old vector with index greater new_index are dirty */
	/* old = &old_list[min_index] here! */
	for (i = (old_index-min_index); i > 0; i--, old++)
	{
		/* skip old clips */
		if (old->status == VCLIP)
			continue;

		/* mark the pixels of the old vector dirty */
		coords = &pixel[old->arg1];
		for (j = (old->arg2 - old->arg1); j > 0; j--)
		{
			coords++;
		}
	}
}

void vector_vh_update(struct osd_bitmap *bitmap,int full_refresh)
{
	int i;
	int temp_x, temp_y;
	point *thenew;

	/* copy parameters */
	vecbitmap = bitmap;
	vecwidth  = bitmap->width;
	vecheight = bitmap->height;

	/* setup scaling */
	temp_x = (1<<(44-vecshift)) / (Machine->drv->visible_area.max_x - Machine->drv->visible_area.min_x);
	temp_y = (1<<(44-vecshift)) / (Machine->drv->visible_area.max_y - Machine->drv->visible_area.min_y);
	if (Machine->orientation & ORIENTATION_SWAP_XY)
	{
		vector_scale_x = temp_x * vecheight;
		vector_scale_y = temp_y * vecwidth;
	}
	else
	{
		vector_scale_x = temp_x * vecwidth;
		vector_scale_y = temp_y * vecheight;
	}

	/* reset clipping area */
	xmin = 0; xmax = vecwidth; ymin = 0; ymax = vecheight;

	/* next call to vector_clear_list() is allowed to swap the lists */
	vector_runs = 0;

	/* mark pixels which are not idential in newlist and oldlist dirty */
	/* the old pixels which get removed are marked dirty immediately,  */
	/* new pixels are recognized by setting new->dirty                 */
	clever_mark_dirty();

	/* clear ALL pixels in the hidden map */
	vector_clear_pixels();

	/* Draw ALL lines into the hidden map. Mark only those lines with */
	/* new->dirty = 1 as dirty. Remember the pixel start/end indices  */
	thenew = new_list;
	for (i = 0; i < new_index; i++)
	{
		if (thenew->status == VCLIP)
			vector_set_clip (thenew->x, thenew->y, thenew->arg1, thenew->arg2);
		else
		{
			thenew->arg1 = p_index;
			vector_draw_to (thenew->x, thenew->y, thenew->col, thenew->intensity, thenew->status);
			thenew->arg2 = p_index;
		}
		thenew++;
	}
}

#endif /* if !(defined xgl) && !(defined xfx) && !(defined svgafx) */
