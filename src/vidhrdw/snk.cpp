#include "driver.h"
#include "vidhrdw/generic.h"

static int bg_tilemap_baseaddr;
#define MAX_VRAM_SIZE (64*64*2)
static int gwar_sprite_placement;
static int k = 0; /*for debugging use */

static int shadows_visible = 0; /* toggles rapidly to fake translucency in ikari warriors */

static void print( struct osd_bitmap *bitmap, int num, int row ){
	char *digit = "0123456789abcdef";
	drawgfx( bitmap,Machine->uifont,
		digit[(num>>8)&0xf],
		0,
		0,0, /* no flip */
		16,row*8+8,
		0,TRANSPARENCY_NONE,0);
	drawgfx( bitmap,Machine->uifont,
		digit[(num>>4)&0xf],
		0,
		0,0, /* no flip */
		16+8,row*8+8,
		0,TRANSPARENCY_NONE,0);
	drawgfx( bitmap,Machine->uifont,
		digit[num&0xf],
		0,
		0,0, /* no flip */
		16+16,row*8+8,
		0,TRANSPARENCY_NONE,0);
}

static void dirty_all( void ){
	int i;
	for( i=0; i<MAX_VRAM_SIZE; i++ ){
		dirtybuffer[i] = 0xff;
	}
}

void update_k( void ){
	if( osd_key_pressed( OSD_KEY_K ) ){
		while( osd_key_pressed( OSD_KEY_K ) );
		k++; dirty_all();
	}
	if( osd_key_pressed( OSD_KEY_J ) ){
		while( osd_key_pressed( OSD_KEY_J ) );
		k--; dirty_all();
	}

	if( osd_key_pressed( OSD_KEY_U) ){
		while( osd_key_pressed( OSD_KEY_U ) );
		k-=8; dirty_all();
	}
	if( osd_key_pressed( OSD_KEY_I ) ){
		while( osd_key_pressed( OSD_KEY_I ) );
		k+=8; dirty_all();
	}
}

static void ViewPalette( struct osd_bitmap *bitmap ){
	int i;
	struct rectangle dest;
	for( i=0; i<1024; i++ ){
		int x0 = (i%16)*7;
		int y0 = (i/16)*8;
		if( y0>=256 ){ x0+=128; y0-=256; }
		dest.min_x = x0;
		dest.min_y = y0;
		dest.max_x = x0+7;
		dest.max_y = y0+7;
		fillbitmap( bitmap, Machine->pens[i], &dest );
	}
}

#define GFX_CHARS			0
#define GFX_TILES			1
#define GFX_SPRITES			2
#define GFX_BIGSPRITES		3

void snk_adpcm_play( int which );

static void adpcm_test( void ){
	if( osd_key_pressed( OSD_KEY_Q ) ){
		while( osd_key_pressed( OSD_KEY_Q ) );
		snk_adpcm_play(0);
	}
	if( osd_key_pressed( OSD_KEY_W ) ){
		while( osd_key_pressed( OSD_KEY_W ) );
		snk_adpcm_play(1);
	}
	if( osd_key_pressed( OSD_KEY_E ) ){
		while( osd_key_pressed( OSD_KEY_E ) );
		snk_adpcm_play(2);
	}
	if( osd_key_pressed( OSD_KEY_R) ){
		while( osd_key_pressed( OSD_KEY_R ) );
		snk_adpcm_play(3);
	}
}

void snk_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom){
	int i;
	int num_colors = 1024;
	for( i=0; i<num_colors; i++ ){
		int bit0,bit1,bit2,bit3;

		colortable[i] = i;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		*palette++ = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[num_colors] >> 0) & 0x01;
		bit1 = (color_prom[num_colors] >> 1) & 0x01;
		bit2 = (color_prom[num_colors] >> 2) & 0x01;
		bit3 = (color_prom[num_colors] >> 3) & 0x01;
		*palette++ = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[2*num_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*num_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*num_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*num_colors] >> 3) & 0x01;
		*palette++ = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		color_prom++;
	}
}

void ikari_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom){
	int i;
	snk_vh_convert_color_prom( palette, colortable, color_prom);

	palette += 6*3;
	/*
		pen#6 is used for translucent shadows;
		we'll just make it dark grey for now
	*/
	for( i=0; i<256; i+=8 ){
		palette[i*3+0] = palette[i*3+1] = palette[i*3+2] = 14;
	}
}

int snk_vh_start( void ){
	dirtybuffer = (unsigned char *)gp2x_malloc( MAX_VRAM_SIZE );
	if( dirtybuffer ){
		tmpbitmap = osd_new_bitmap( 512, 512, Machine->scrbitmap->depth );
		if( tmpbitmap ){
			fast_memset( dirtybuffer, 0xff, MAX_VRAM_SIZE  );

			gwar_sprite_placement = (strcmp(Machine->gamedrv->name,"gwar")==0);

			if( strcmp(Machine->gamedrv->name, "ikarijp")==0 ||
				strcmp(Machine->gamedrv->name, "ikarijpb")==0 ){
				bg_tilemap_baseaddr = 0xd000; /* special case */
			}
			else {
				bg_tilemap_baseaddr = 0xd800;
			}
			shadows_visible = 1;
			return 0;
		}
		gp2x_free( dirtybuffer );
	}
	return 1;
}

void snk_vh_stop( void ){
	osd_free_bitmap( tmpbitmap );
	gp2x_free( dirtybuffer );
}

/**************************************************************************************/

static void tnk3_draw_background( struct osd_bitmap *bitmap, int scrollx, int scrolly ){
	const struct rectangle *clip = &Machine->drv->visible_area;
	const struct GfxElement *gfx = Machine->gfx[GFX_TILES];
	int offs;

	if( osd_key_pressed(OSD_KEY_G ) ) dirty_all();

	for( offs=0; offs<64*64*2; offs+=2 ){
		int tile_number = videoram[offs];
		unsigned char attributes = videoram[offs+1];

		if( tile_number!=dirtybuffer[offs] || attributes != dirtybuffer[offs+1] ){
			int sy = ((offs/2)%64)*8;
			int sx = ((offs/2)/64)*8;
			int color = 0xe;
			if( (k&0xf)==(attributes&0xf) && osd_key_pressed(OSD_KEY_G) ) color = random()&0xff;

			dirtybuffer[offs] = tile_number;
			dirtybuffer[offs+1] = attributes;

			tile_number += 256*((attributes>>4)&0x3);

			if( attributes&0xc0 ) color = k;

			drawgfx( tmpbitmap,gfx,
				tile_number,
				color,
				0,0, /* no flip */
				sx,sy,
				0,TRANSPARENCY_NONE,0);
		}
	}

	{
		copyscrollbitmap(bitmap,tmpbitmap,
			1,&scrollx,1,&scrolly,
			clip,
			TRANSPARENCY_NONE,0);
	}
}

static void aso_draw_background( struct osd_bitmap *bitmap, int scrollx, int scrolly ){
	const struct rectangle *clip = &Machine->drv->visible_area;
	const struct GfxElement *gfx = Machine->gfx[GFX_TILES];
	static int bank = 0;
	int offs;

	if( osd_key_pressed( OSD_KEY_B ) ){
		while( osd_key_pressed( OSD_KEY_B ) );
		bank = (bank+1)&3;
	}

	for( offs=0; offs<64*64; offs++ ){
		int tile_number = videoram[offs]+bank*256;

		int sy = (offs%64)*8;
		int sx = (offs/64)*8;
		int color = 0;

		drawgfx( tmpbitmap,gfx,
			tile_number,
			color,
			0,0, /* no flip */
			sx,sy,
			0,TRANSPARENCY_NONE,0);
	}

	{
		copyscrollbitmap(bitmap,tmpbitmap,
			1,&scrollx,1,&scrolly,
			clip,
			TRANSPARENCY_NONE,0);
	}

	if( osd_key_pressed( OSD_KEY_S ) ){
		FILE *f = fopen("dump","wb");
		if( f ){
			fwrite( Machine->memory_region[0], 0x10000, 1, f );
			fclose( f );
		}
		while( osd_key_pressed( OSD_KEY_S ) );
	}
}

static void tnk3_draw_text( struct osd_bitmap *bitmap, int bank, unsigned char *source ){
	const struct GfxElement *gfx = Machine->gfx[GFX_CHARS];
	const struct rectangle *clip = &Machine->drv->visible_area;
	int color = 0x18;
	int offs;
	bank*=256;

	for( offs = 0;offs <0x400; offs++ ){
		drawgfx( bitmap, gfx,
			source[offs]+bank,
			color,
			0,0, /* no flip */
			16+(offs/32)*8,(offs%32)*8+8,
			clip,
			TRANSPARENCY_PEN,15 );
	}
}

static void tnk3_draw_status( struct osd_bitmap *bitmap, int bank, unsigned char *source ){
	const struct rectangle *clip = &Machine->drv->visible_area;
	int color = 0x18;

	int offs;
	bank *= 256;
	for( offs = 0; offs<64; offs++ ){
		int tile_number = source[offs+30*32];
		int sy = (offs % 32)*8+8;
		int sx = (offs / 32)*8;

		drawgfx(bitmap,Machine->gfx[GFX_CHARS],
			tile_number+bank,
			color,
			0,0, /* no flip */
			sx,sy,
			clip,TRANSPARENCY_NONE,0);

		tile_number = source[offs];
		sx += 34*8;

		drawgfx(bitmap,Machine->gfx[GFX_CHARS],
			tile_number+bank,
			color,
			0,0, /* no flip */
			sx,sy,
			clip,TRANSPARENCY_NONE,0);
	}
}

void tnk3_draw_sprites( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	const unsigned char *source = spriteram;
	const unsigned char *finish = source+60*4;

	struct rectangle clip = Machine->drv->visible_area;

	while( source<finish ){
		int attributes = source[3]; /* YBBX.CCCC */
		int tile_number = source[1];
		int sy = source[0] + ((attributes&0x10)?256:0) - yscroll;
		int sx = source[2] + ((attributes&0x80)?256:0) - xscroll;
		int color = attributes&0xf;

		if( attributes&0x40 ) tile_number += 256;
		if( attributes&0x20 ) tile_number += 512;

		if( (k&0xf)==color || !osd_key_pressed(OSD_KEY_H) )

		drawgfx(bitmap,Machine->gfx[GFX_SPRITES],
			tile_number,
			k,
			0,0,
			(256-sx)&0x1ff,sy&0x1ff,
			&clip,TRANSPARENCY_PEN,7);

		source+=4;
	}
}

void aso_draw_sprites( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	const unsigned char *source = spriteram;
	const unsigned char *finish = source+60*4;

	struct rectangle clip = Machine->drv->visible_area;

	while( source<finish ){
		int attributes = source[3]; /* YBBX.CCCC */
		int tile_number = source[1];
		int sy = source[0] + ((attributes&0x10)?256:0) - yscroll;
		int sx = source[2] + ((attributes&0x80)?256:0) - xscroll;
		int color = attributes&0xf;

		if( !(attributes&0x20) ) tile_number += 512;
		if( attributes&0x40 ) tile_number += 256;

		if( (k&0xf)==color || !osd_key_pressed(OSD_KEY_H) )
		drawgfx(bitmap,Machine->gfx[GFX_SPRITES],
			tile_number,
			k,
			0,0,
			(256-sx)&0x1ff,sy&0x1ff,
			&clip,TRANSPARENCY_PEN,7);

		source+=4;
	}
}


void aso_vh_screenrefresh( struct osd_bitmap *bitmap, int full_refresh ){
	unsigned char *ram = Machine->memory_region[Machine->drv->cpu[0].memory_region];
	int attributes = ram[0xc800];

	/* to be moved to memmap */
	spriteram = &ram[0xe000];
	videoram = &ram[0xe800];

	update_k();
	if( osd_key_pressed( OSD_KEY_A ) ){
		ViewPalette( bitmap );
		return;
	}

	{
		int scrolly = -8+ram[0xcb00]+((attributes&0x10)?256:0);
		int scrollx = 0x1e*8 + ram[0xcc00]+((attributes&0x02)?256:0);
		aso_draw_background( bitmap, -scrollx, -scrolly );
	}

	{
		int scrolly = -8+0x11+ram[0xc900] + ((attributes&0x08)?256:0);
		int scrollx = 0x1e + ram[0xca00] + ((attributes&0x01)?256:0);
		aso_draw_sprites( bitmap, scrollx, scrolly );
	}

	{
		int bank = (attributes&0x40)?1:0;
		tnk3_draw_text( bitmap, bank, &ram[0xf800] );
		tnk3_draw_status( bitmap, bank, &ram[0xfc00] );
	}

	print( bitmap, k, 2 );
}

void tnk3_vh_screenrefresh( struct osd_bitmap *bitmap, int full_refresh ){
	unsigned char *ram = Machine->memory_region[Machine->drv->cpu[0].memory_region];
	int attributes = ram[0xc800];

	/* to be moved to memmap */
	spriteram = &ram[0xd000];
	videoram = &ram[0xd800];

	update_k();
	if( osd_key_pressed( OSD_KEY_A ) ){
		ViewPalette( bitmap );
		return;
	}

	{
		int scrolly =  -8+ram[0xcb00]+((attributes&0x10)?256:0);
		int scrollx = -16+ram[0xcc00]+((attributes&0x02)?256:0);
		tnk3_draw_background( bitmap, -scrollx, -scrolly );
	}

	{
		int scrolly =  8+ram[0xc900] + ((attributes&0x08)?256:0);
		int scrollx = 30+ram[0xca00] + ((attributes&0x01)?256:0);
		tnk3_draw_sprites( bitmap, scrollx, scrolly );
	}

	{
		int bank = (attributes&0x40)?1:0;
		tnk3_draw_text( bitmap, bank, &ram[0xf800] );
		tnk3_draw_status( bitmap, bank, &ram[0xfc00] );
	}

	print( bitmap, k, 2 );
}

/**************************************************************************************/

static void ikari_draw_background( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	const struct GfxElement *gfx = Machine->gfx[GFX_TILES];
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][bg_tilemap_baseaddr];

	int offs;
	for( offs=0; offs<32*32*2; offs+=2 ){
		int tile_number = source[offs];
		unsigned char attributes = source[offs+1];

		if( tile_number!=dirtybuffer[offs] ||
			attributes != dirtybuffer[offs+1] ){

			int sy = ((offs/2)%32)*16;
			int sx = ((offs/2)/32)*16;

			dirtybuffer[offs] = tile_number;
			dirtybuffer[offs+1] = attributes;

			tile_number+=256*(attributes&0x3);

			drawgfx(tmpbitmap,gfx,
				tile_number,
				(attributes>>4), /* color */
				0,0, /* no flip */
				sx,sy,
				0,TRANSPARENCY_NONE,0);
		}
	}

	{
		struct rectangle clip = Machine->drv->visible_area;
		clip.min_x += 16;
		clip.max_x -= 16;
		copyscrollbitmap(bitmap,tmpbitmap,
			1,&xscroll,1,&yscroll,
			&clip,
			TRANSPARENCY_NONE,0);
	}
}

static void ikari_draw_text( struct osd_bitmap *bitmap ){
	const struct rectangle *clip = &Machine->drv->visible_area;
	const struct GfxElement *gfx = Machine->gfx[GFX_CHARS];
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xf800];

	int offs;
	for( offs = 0;offs <0x400; offs++ ){
		int tile_number = source[offs];
		if( source[offs] != 0x20 ){ /* skip spaces */
			int sy = (offs % 32)*8+8;
			int sx = (offs / 32)*8+8;

			drawgfx(bitmap,gfx,
				tile_number+256,
				8, /* color */
				0,0, /* no flip */
				sx,sy,
				clip,TRANSPARENCY_PEN,15);
		}
	}
}

static void ikari_draw_status( struct osd_bitmap *bitmap ){
	/*	this is drawn above and below the main display */

	const struct rectangle *clip = &Machine->drv->visible_area;
	const struct GfxElement *gfx = Machine->gfx[GFX_CHARS];
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xfc00];

	int offs;
	for( offs = 0; offs<64; offs++ ){
		int tile_number = source[offs+30*32]+256;
		int sy = 20+(offs % 32)*8 - 16;
		int sx = (offs / 32)*8;

		drawgfx(bitmap,gfx,
			tile_number,
			8, /* color */
			0,0, /* no flip */
			sx,sy,
			clip,TRANSPARENCY_NONE,0);

		tile_number = source[offs]+256;
		sx += 33*8;

		drawgfx(bitmap,gfx,
			tile_number,
			8, /* color */
			0,0, /* no flip */
			sx,sy,
			clip,TRANSPARENCY_NONE,0);
	}
}

static void ikari_draw_sprites_16x16( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	int transp_mode = shadows_visible?TRANSPARENCY_PEN:TRANSPARENCY_PENS;
	int transp_param = shadows_visible?7:(1<<7)|(1<<6);

	int which;
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xe800];

	struct rectangle clip = Machine->drv->visible_area;
	clip.min_x += 16;
	clip.max_x -= 16;

	for( which = 0; which < 50*4; which+=4 ){
		int attributes = source[which+3]; /* YBBX.CCCC */
		int tile_number = source[which+1] + ((attributes&0x60)<<3);
		int sy = - yscroll + source[which]  +((attributes&0x10)?256:0);
		int sx =   xscroll - source[which+2]+((attributes&0x80)?0:256);

		drawgfx(bitmap,Machine->gfx[GFX_SPRITES],
			tile_number,
			attributes&0xf, /* color */
			0,0, /* flip */
			(sx&0x1ff),(sy&0x1ff),
			&clip,transp_mode,transp_param);
	}
}

static void ikari_draw_sprites_32x32( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	int transp_mode = shadows_visible?TRANSPARENCY_PEN:TRANSPARENCY_PENS;
	int transp_param = shadows_visible?7:(1<<7)|(1<<6);

	int which;
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xe000];

	struct rectangle clip = Machine->drv->visible_area;
	clip.min_x += 16;
	clip.max_x -= 16;

	for( which = 0; which < 25*4; which+=4 ){
		int attributes = source[which+3];
		int tile_number = source[which+1];
		int sy = - yscroll + source[which]   + ((attributes&0x10)?256:0);
		int sx =   xscroll - source[which+2] + ((attributes&0x80)?0:256);
		if( attributes&0x40 ) tile_number += 256;

		drawgfx( bitmap,Machine->gfx[GFX_BIGSPRITES],
			tile_number,
			attributes&0xf, /* color */
			0,0, /* flip */
			(sx&0x1ff),(sy&0x1ff),
			&clip,transp_mode,transp_param );
	}
}

void ikari_vh_screenrefresh( struct osd_bitmap *bitmap, int full_refresh){
	const unsigned char *ram = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	shadows_visible = !shadows_visible;

	adpcm_test();
	update_k();

	{
		int attributes = ram[0xc900];
		int scrolly =  8-ram[0xc800] - ((attributes&0x01)?256:0);
		int scrollx = 16-ram[0xc880] - ((attributes&0x02)?256:0);
		ikari_draw_background( bitmap, scrollx, scrolly );
	}

	{
		int attributes = ram[0xcd00];
		int sp16_scrolly =  8 + ram[0xca00] + ((attributes&0x04)?256:0);
		int sp16_scrollx = 23 + ram[0xca80] + ((attributes&0x10)?256:0);

		int sp32_scrolly = 24 + ram[0xcb00] + ((attributes&0x08)?256:0);
		int sp32_scrollx =  7 + ram[0xcb80] + ((attributes&0x20)?256:0);

		ikari_draw_sprites_16x16( bitmap, sp16_scrollx, sp16_scrolly );
		ikari_draw_sprites_32x32( bitmap, sp32_scrollx, sp32_scrolly );
	}

	ikari_draw_text( bitmap );
	ikari_draw_status( bitmap );
}

/**************************************************************/

static void tdfever_draw_background( struct osd_bitmap *bitmap,
		int xscroll, int yscroll )
{
	const struct GfxElement *gfx = Machine->gfx[GFX_TILES];
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xd000];

	int offs;
	for( offs=0; offs<32*32*2; offs+=2 ){
		int tile_number = source[offs];
		unsigned char attributes = source[offs+1];

		if( tile_number!=dirtybuffer[offs] ||
			attributes != dirtybuffer[offs+1] ){

			int sy = ((offs/2)%32)*16;
			int sx = ((offs/2)/32)*16;

			int color = (attributes>>4); /* color */

			dirtybuffer[offs] = tile_number;
			dirtybuffer[offs+1] = attributes;

			tile_number+=256*(attributes&0xf);

			drawgfx(tmpbitmap,gfx,
				tile_number,
				color,
				0,0, /* no flip */
				sx,sy,
				0,TRANSPARENCY_NONE,0);
		}
	}

	{
		struct rectangle clip = Machine->drv->visible_area;
		copyscrollbitmap(bitmap,tmpbitmap,
			1,&xscroll,1,&yscroll,
			&clip,
			TRANSPARENCY_NONE,0);
	}
}

static void tdfever_draw_sprites( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	const struct GfxElement *gfx = Machine->gfx[GFX_SPRITES];
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xe000];

	int which;

	struct rectangle clip = Machine->drv->visible_area;

	for( which = 0; which < 32*4; which+=4 ){
		int attributes = source[which+3];
		int tile_number = source[which+1] + 8*(attributes&0x60);

		int sy = - yscroll + source[which];
		int sx = xscroll - source[which+2];
		if( attributes&0x10 ) sy += 256;
		if( attributes&0x80 ) sx -= 256;

		sx &= 0x1ff; if( sx>512-32 ) sx-=512;
		sy &= 0x1ff; if( sy>512-32 ) sy-=512;

		drawgfx(bitmap,gfx,
			tile_number,
			(attributes&0xf), /* color */
			0,0, /* no flip */
			sx,sy,
			&clip,TRANSPARENCY_PEN,15);
	}
}

static void tdfever_draw_text( struct osd_bitmap *bitmap, int attributes, int dx, int dy ){
	int bank = attributes>>4;
	int color = attributes&0xf;

	const struct rectangle *clip = &Machine->drv->visible_area;
	const struct GfxElement *gfx = Machine->gfx[GFX_CHARS];

	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xf800];

	int offs;

	int bank_offset = bank*256;

	for( offs = 0;offs <0x800; offs++ ){
		int tile_number = source[offs];
		int sy = dx+(offs % 32)*8;
		int sx = dy+(offs / 32)*8;

		if( source[offs] != 0x20 ){
			drawgfx(bitmap,gfx,
				tile_number + bank_offset,
				color,
				0,0, /* no flip */
				sx,sy,
				clip,TRANSPARENCY_PEN,15);
		}
	}
}

void tdfever_vh_screenrefresh( struct osd_bitmap *bitmap, int fullrefresh ){
	const unsigned char *ram = Machine->memory_region[Machine->drv->cpu[0].memory_region];
	adpcm_test();

	{
		unsigned char bg_attributes = ram[0xc880];
		int bg_scroll_y = -30 - ram[0xc800] - ((bg_attributes&0x01)?256:0);
		int bg_scroll_x = 141 - ram[0xc840] - ((bg_attributes&0x02)?256:0);
		tdfever_draw_background( bitmap, bg_scroll_x, bg_scroll_y );
	}

	{
		unsigned char sprite_attributes = ram[0xc900];
		int sprite_scroll_y =   65 + ram[0xc980] + ((sprite_attributes&0x80)?256:0);
		int sprite_scroll_x = -135 + ram[0xc9c0] + ((sprite_attributes&0x40)?256:0);
		tdfever_draw_sprites( bitmap, sprite_scroll_x, sprite_scroll_y );
	}

	{
		unsigned char text_attributes = ram[0xc8c0];
		tdfever_draw_text( bitmap, text_attributes, 0,0 );
	}
}

static void gwar_draw_sprites_16x16( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	const struct GfxElement *gfx = Machine->gfx[GFX_SPRITES];
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xe800];
	int which;

	const struct rectangle *clip = &Machine->drv->visible_area;

	for( which=0; which<64*4; which+=4 ){
		int attributes = source[which+3]; /* YBBX.BCCC */
		int tile_number = source[which+1];
		int sy = -xscroll + source[which];
		int sx =  yscroll - source[which+2];
		if( attributes&0x10 ) sy += 256;
		if( attributes&0x80 ) sx -= 256;

		if( attributes&0x08 ) tile_number += 256;
		if( attributes&0x20 ) tile_number += 512;
		if( attributes&0x40 ) tile_number += 1024;

		sy &= 0x1ff; if( sy>512-16 ) sy-=512;
		sx = (-sx)&0x1ff; if( sx>512-16 ) sx-=512;

		drawgfx(bitmap,gfx,
			tile_number,
			(attributes&7), /* color */
			0,0, /* flip */
			sx,sy,
			clip,TRANSPARENCY_PEN,15 );
	}
}

void gwar_draw_sprites_32x32( struct osd_bitmap *bitmap, int xscroll, int yscroll ){
	const struct GfxElement *gfx = Machine->gfx[GFX_BIGSPRITES];
	const unsigned char *source = &Machine->memory_region[Machine->drv->cpu[0].memory_region][0xe000];

	const struct rectangle *clip = &Machine->drv->visible_area;

	int which;
	for( which=0; which<(32)*4; which+=4 ){
		int attributes = source[which+3];
		int tile_number = source[which+1] + 8*(attributes&0x60);

		int sy = - xscroll + source[which];
		int sx = yscroll - source[which+2];
		if( attributes&0x10 ) sy += 256;
		if( attributes&0x80 ) sx -= 256;

		sy = (sy&0x1ff);
		sx = ((-sx)&0x1ff);
		if( sy>512-32 ) sy-=512;
		if( sx>512-32 ) sx-=512;

		drawgfx(bitmap,gfx,
			tile_number,
			(attributes&0xf), /* color */
			0,0, /* no flip */
			sx,sy,
			clip,TRANSPARENCY_PEN,15);
	}
}

void gwar_vh_screenrefresh( struct osd_bitmap *bitmap, int full_refresh ){
	const unsigned char *ram = Machine->memory_region[Machine->drv->cpu[0].memory_region];
	unsigned char bg_attributes = ram[0xc880];

	adpcm_test();
	update_k();

	{
		int bg_scroll_y = - ram[0xc800] - ((bg_attributes&0x01)?256:0);
		int bg_scroll_x = 16 - ram[0xc840] - ((bg_attributes&0x02)?256:0);
		tdfever_draw_background( bitmap, bg_scroll_x, bg_scroll_y );
	}

	{
		int sp16_y = ram[0xc900]+15;
		int sp16_x = ram[0xc940]+8;
		int sp32_y = ram[0xc980]+31;
		int sp32_x = ram[0xc9c0]+8;

		if( gwar_sprite_placement ){ /* gwar */
			if( bg_attributes&0x10 ) sp16_y += 256;
			if( bg_attributes&0x40 ) sp16_x += 256;
			if( bg_attributes&0x20 ) sp32_y += 256;
			if( bg_attributes&0x80 ) sp32_x += 256;
		}
		else{ /* psychos, bermudet, chopper1... */
			unsigned char sp_attributes = ram[0xca80];
			if( sp_attributes&0x04 )sp16_y += 256;
			if( sp_attributes&0x08 )sp32_y += 256;
			if( sp_attributes&0x10 )sp16_x += 256;
			if( sp_attributes&0x20 )sp32_x += 256;
		}

		if( osd_key_pressed(OSD_KEY_F) ){
			gwar_draw_sprites_16x16( bitmap, sp16_y, sp16_x );
			gwar_draw_sprites_32x32( bitmap, sp32_y, sp32_x );
		}
		else {
			gwar_draw_sprites_32x32( bitmap, sp32_y, sp32_x );
			gwar_draw_sprites_16x16( bitmap, sp16_y, sp16_x );
		}
	}

	{
		unsigned char text_attributes = ram[0xc8c0];
		tdfever_draw_text( bitmap, text_attributes,0,0 );
	}
}
