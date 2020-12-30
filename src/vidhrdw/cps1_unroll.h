
#define cps1_draw_gfx_init(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	int i, j; \
	dword *src; \
	const unsigned short *paldata; \
	(TPENS)=(~(TPENS)) & 0xffff; \
	if ( (CODE) > (MAX) || !((TPENS) & (PUSAGE)[(CODE)])) \
		return;

#define cps1_draw_gfx_init_without_tpens(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	int i, j; \
	dword *src; \
	const unsigned short *paldata; \
	if ( (CODE) > (MAX) || !((TPENS) & (PUSAGE)[(CODE)])) \
		return;

#define cps1_draw_gfx_swapxy(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	{ \
		int temp; \
		temp=(SX); \
		(SX)=(SY); \
		(SY)=bitmap_height-temp-(SIZE); \
	} 


#define cps1_draw_gfx_flipscr(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	{ \
		(SX)=bitmap_width-(SX)-(SIZE); \
		(SY)=bitmap_height-(SY)-(SIZE); \
	}

#ifndef CPS1_BITMAP_PATCH
#define cps1_draw_gfx_precheck(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
       	if ((SX)<0 || (SX) > bitmap_width-(SIZE) || (SY)<0 || (SY)>bitmap_height-(SIZE))
#else
#ifdef GP2X
#define cps1_draw_gfx_precheck(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	if (((gp2x_rotate) && ((SX)<0 || (SX) > bitmap_width-(SIZE) || (SY)<0 || (SY)>bitmap_height-(SIZE))) || \
		((!gp2x_rotate) && ((SX)<bitmap_width_min-(SIZE) || (SX) > bitmap_width_max+(SIZE) || (SY)<bitmap_height_min-(SIZE) || (SY)>bitmap_height_max+(SIZE) )))
#else
#define cps1_draw_gfx_precheck(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	if ((SX)<bitmap_width_min-(SIZE) || (SX) > bitmap_width_max+(SIZE) || (SY)<bitmap_height_min-(SIZE) || (SY)>bitmap_height_max+(SIZE) )
#endif
#endif

#define cps1_draw_gfx_check(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_precheck((GFX),(CODE),(COLOR),(SX),(SY),(TPENS),(PUSAGE),(SIZE),(MAX),(DELTA),(SRCDELTA)) \
		return; \
	src = cps1_gfx+(CODE)*(DELTA); \
	paldata = &(GFX)->colortable[(GFX)->color_granularity * (COLOR)]; 


#define cps1_draw_gfx_orient_flipy_flipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	(SY)+=(SIZE); \
	(SX)+=(SIZE); \
	for (i=0; i<(SIZE); i++) \
	{ \
		int ny=(SY); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned char *bm; \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>24)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>20)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>16)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>12)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>8)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>4)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=dwval&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      src++; \
		} \
		(SX)--; \
		src+=(SRCDELTA); \
	}

#define cps1_draw_gfx_orient_flipy_noflipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	(SY)+=(SIZE); \
	for (i=0; i<(SIZE); i++) \
	{ \
		int ny=(SY); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned char *bm; \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>24)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>20)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>16)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>12)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>8)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>4)&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=dwval&0x0f; \
		      bm=(unsigned char *)bitmap_line[--ny]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      src++; \
		} \
		(SX)++; \
		src+=(SRCDELTA); \
	}

#define cps1_draw_gfx_orient_noflipy_flipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	(SX)+=(SIZE); \
	for (i=0; i<(SIZE); i++) \
	{ \
		int ny=(SY); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned char *bm; \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>24)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>20)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>16)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>12)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>8)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=(dwval>>4)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      n=dwval&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) bm[-1]=paldata[n]; \
		      src++; \
		} \
		(SX)--; \
		src+=(SRCDELTA); \
	}

#define cps1_draw_gfx_orient_noflipy_noflipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	for (i=0; i<(SIZE); i++) \
	{ \
		int ny=(SY); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned char *bm; \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>24)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>20)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>16)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>12)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>8)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>4)&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=dwval&0x0f; \
		      bm=(unsigned char *)bitmap_line[ny++]+(SX); \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      src++; \
		} \
		(SX)++; \
		src+=(SRCDELTA); \
	}


#define cps1_draw_gfx_norient_flipy_flipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	(SY)+=(SIZE)-1; \
	(SX)+=(SIZE); \
	for (i=0; i<(SIZE); i++) \
	{ \
		register unsigned char *bm=(unsigned char *)bitmap_line[(SY)-i]+(SX); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>24)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>20)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>16)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>12)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>8)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>4)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=dwval&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      src++; \
		} \
		src+=(SRCDELTA); \
	}

#define cps1_draw_gfx_norient_flipy_noflipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	(SY)+=(SIZE)-1; \
	for (i=0; i<(SIZE); i++) \
	{ \
		register unsigned char *bm=(unsigned char *)bitmap_line[(SY)-i]+(SX); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>24)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>20)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>16)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>12)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>8)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>4)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=dwval&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      src++; \
		} \
		src+=(SRCDELTA); \
	}


#define cps1_draw_gfx_norient_noflipy_flipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	(SX)+=(SIZE); \
	for (i=0; i<(SIZE); i++) \
	{ \
		register unsigned char *bm=(unsigned char *)bitmap_line[(SY)+i]+(SX); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>24)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>20)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>16)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>12)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>8)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=(dwval>>4)&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      n=dwval&0x0f; \
		      bm--; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      src++; \
		} \
		src+=(SRCDELTA); \
	}

#define cps1_draw_gfx_norient_noflipy_noflipx(GFX,CODE,COLOR,SX,SY,TPENS,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	for (i=0; i<(SIZE); i++) \
	{ \
		register unsigned char *bm=(unsigned char *)bitmap_line[(SY)+i]+(SX); \
		for (j=0; j<((SIZE)>>3); j++) \
		{ \
		      register unsigned dwval=*src; \
		      register unsigned n=(dwval>>28)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>24)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>20)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>16)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>12)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>8)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=(dwval>>4)&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      n=dwval&0x0f; \
		      if ((TPENS) & (0x01 << n)) *bm=paldata[n]; \
		      bm++; \
		      src++; \
		} \
		src+=(SRCDELTA); \
	 }

#define DEFAULT_TPENS ((~(0x8000)) & 0xffff)


#define cps1_draw_norient_noflipscr(TITLE,INIT,FLIPY,FLIPX,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
static void cps1_draw_##TITLE##_norient_noflipscr_##FLIPY##_##FLIPX(const struct GfxElement *gfx, unsigned int code, int color, int sx, int sy, int tpens) \
{ \
	INIT(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_check(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_norient_##FLIPY##_##FLIPX(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
}

#define cps1_draw_norient_flipscr(TITLE,INIT,FLIPY,FLIPX,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
static void cps1_draw_##TITLE##_norient_flipscr_##FLIPY##_##FLIPX(const struct GfxElement *gfx, unsigned int code, int color, int sx, int sy, int tpens) \
{ \
	INIT(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_flipscr(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_check(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_norient_##FLIPY##_##FLIPX(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
}

#define cps1_draw_orient_noflipscr(TITLE,INIT,FLIPY,FLIPX,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
static void cps1_draw_##TITLE##_orient_noflipscr_##FLIPY##_##FLIPX(const struct GfxElement *gfx, unsigned int code, int color, int sx, int sy, int tpens) \
{ \
	INIT(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_swapxy(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_check(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_orient_##FLIPY##_##FLIPX(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
}

#define cps1_draw_orient_flipscr(TITLE,INIT,FLIPY,FLIPX,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
static void cps1_draw_##TITLE##_orient_flipscr_##FLIPY##_##FLIPX(const struct GfxElement *gfx, unsigned int code, int color, int sx, int sy, int tpens) \
{ \
	INIT(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_swapxy(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_flipscr(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_check(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_gfx_orient_##FLIPY##_##FLIPX(gfx,code,color,sx,sy,tpens,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
}


#define cps1_draw_all_1(TITLE,INIT,ORIENT,FLIPSCR,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_##ORIENT##_##FLIPSCR(TITLE,INIT,noflipy,noflipx,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_##ORIENT##_##FLIPSCR(TITLE,INIT,noflipy,flipx,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_##ORIENT##_##FLIPSCR(TITLE,INIT,flipy,noflipx,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_##ORIENT##_##FLIPSCR(TITLE,INIT,flipy,flipx,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) 
	
#define cps1_draw_all(TITLE,INIT,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_all_1(TITLE,INIT,norient,noflipscr,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_all_1(TITLE,INIT,norient,flipscr,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_all_1(TITLE,INIT,orient,noflipscr,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
	cps1_draw_all_1(TITLE,INIT,orient,flipscr,PUSAGE,SIZE,MAX,DELTA,SRCDELTA) \
typedef void (*cps1_draw_##TITLE##_func)(const struct GfxElement *, unsigned int,int,int,int,int); \
cps1_draw_##TITLE##_func cps1_draw_##TITLE##_array[16]= \
{ \
 cps1_draw_##TITLE##_norient_noflipscr_noflipy_noflipx, \
 cps1_draw_##TITLE##_norient_noflipscr_noflipy_flipx, \
 cps1_draw_##TITLE##_norient_noflipscr_flipy_noflipx, \
 cps1_draw_##TITLE##_norient_noflipscr_flipy_flipx, \
 cps1_draw_##TITLE##_norient_flipscr_noflipy_noflipx, \
 cps1_draw_##TITLE##_norient_flipscr_noflipy_flipx, \
 cps1_draw_##TITLE##_norient_flipscr_flipy_noflipx, \
 cps1_draw_##TITLE##_norient_flipscr_flipy_flipx, \
 cps1_draw_##TITLE##_orient_noflipscr_flipy_noflipx, \
 cps1_draw_##TITLE##_orient_noflipscr_noflipy_noflipx, \
 cps1_draw_##TITLE##_orient_noflipscr_flipy_flipx, \
 cps1_draw_##TITLE##_orient_noflipscr_noflipy_flipx, \
 cps1_draw_##TITLE##_orient_flipscr_noflipy_flipx, \
 cps1_draw_##TITLE##_orient_flipscr_flipy_flipx, \
 cps1_draw_##TITLE##_orient_flipscr_noflipy_noflipx, \
 cps1_draw_##TITLE##_orient_flipscr_flipy_noflipx \
};\
cps1_draw_##TITLE##_func *cps1_draw_##TITLE##_pointer=&cps1_draw_##TITLE##_array[0];

cps1_draw_all(scroll1,cps1_draw_gfx_init_without_tpens,cps1_char_pen_usage,8,cps1_max_char,16,1)
cps1_draw_all(tile16,cps1_draw_gfx_init,cps1_tile16_pen_usage,16,cps1_max_tile16,16*2,0)
cps1_draw_all(tile32,cps1_draw_gfx_init,cps1_tile32_pen_usage,32,cps1_max_tile32,16*2*4,0)



INLINE void adjust_draw_gfx_pointers(void)
{
	int pos=orientation_swap_xy|cps1_flip_screen;
	cps1_draw_scroll1_pointer=&cps1_draw_scroll1_array[pos];
	cps1_draw_tile16_pointer=&cps1_draw_tile16_array[pos];
	cps1_draw_tile32_pointer=&cps1_draw_tile32_array[pos];
}

#define cps1_draw_scroll1(code,color,flipx,flipy,sx,sy) \
{ \
 	(cps1_draw_scroll1_pointer[((flipx)>>5)|((flipy)>>5)])((const struct GfxElement *)Machine->gfx[0],code,color,sx,sy,DEFAULT_TPENS);  \
}
#define cps1_draw_tile16(gfx,code,color,flipx,flipy,sx,sy,tpens) \
{ \
 	(cps1_draw_tile16_pointer[((flipx)>>5)|((flipy)>>5)])(gfx,code,color,sx,sy,tpens); \
}

#define cps1_draw_tile32(gfx,code,color,flipx,flipy,sx,sy,tpens) \
{ \
 	(cps1_draw_tile32_pointer[((flipx)>>5)|((flipy)>>5)])(gfx,code,color,sx,sy,tpens);  \
}


