#ifdef DREAMCAST
#include<kos.h>
#endif

#ifdef USE_FILECACHE

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "minimal.h"

#define FILECACHE_SIZE (1920*1024)
#define FILECACHE_SPLIT 32
#define FILECACHE_TILE (16*1024)
#define FILECACHE_N (FILECACHE_SIZE/FILECACHE_TILE)

static char backup_path[64];
#ifndef DREAMCAST
static unsigned char fc_real_buff[FILECACHE_SIZE];
static const unsigned fc_buff=(unsigned)&fc_real_buff;
#else
//static const unsigned fc_buff=0x0820000;
static const unsigned fc_buff=0xa0820000;
#endif
static short fc_used[FILECACHE_N];
static unsigned short fc_time[FILECACHE_N];
static FILE* fc_file=NULL;
static long fc_times=0;
static long fc_size=0;
static long fc_pos=0;
static int fc_prefetching=0;
static int fc_initted=0;

void unzip_reset_malloc(void);

static int filecache_read_one_block(int n)
{
	int i,a=0,at=0x1FFFFFFF;
	for(i=0;i<FILECACHE_N;i++)
		if (fc_used[i]==n)
		{
			fc_times++;
			fc_time[i]=fc_times;
			return 0;
		}
		else
		if (fc_time[i]<at)
		{
			a=i;
			at=fc_time[i];
		}

	fseek(fc_file,n*FILECACHE_TILE,SEEK_SET);
	int split;
	for(split=0;split<(FILECACHE_TILE/FILECACHE_SPLIT);split++)
	{
#ifdef DREAMCAST
	dcache_flush_range(fc_buff+(a*FILECACHE_TILE)+(split*FILECACHE_SPLIT),FILECACHE_SPLIT);
//	dcache_inval_range(fc_buff+(a*FILECACHE_TILE)+(split*FILECACHE_SPLIT),FILECACHE_SPLIT);
#endif
		fread((void *)(fc_buff+(a*FILECACHE_TILE)+(split*FILECACHE_SPLIT)),FILECACHE_SPLIT,1,fc_file);
#ifdef DREAMCAST

	{
		vuint32 const *g2_fifo = (vuint32*)0xa05f688c;
		int g2_i;
		for (g2_i=0; g2_i<0x1800; g2_i++)
			if (!(*g2_fifo & 0x11)) break;
	}
	dcache_inval_range(fc_buff+(a*FILECACHE_TILE)+(split*FILECACHE_SPLIT),FILECACHE_SPLIT);
//	dcache_flush_range(fc_buff+(a*FILECACHE_TILE)+(split*FILECACHE_SPLIT),FILECACHE_SPLIT);
#endif
	}
	fc_times++;
	fc_time[a]=fc_times;
	fc_used[a]=n;
	return 0;
}

static void filecache_read_block(int n)
{
	filecache_read_one_block(n);
	if (!fc_prefetching)
	{
		int max=fc_size/FILECACHE_TILE;
		n++; 
		if (n<max)
		{
//			fc_times-=8;
			filecache_read_one_block(n);
/*
			n++;
			if (n<max)
			{
				filecache_read_one_block(n);
				n++; 
				if (n<max)
					filecache_read_one_block(n);
			}
*/
//			fc_times+=6;
		}
	}
}

static void *filecache_copy_block(void *buff, unsigned offset, int n, int len)
{
	if (buff)
	{
		int i;
		for(i=0;i<FILECACHE_N;i++)
			if (fc_used[i]==n)
			{
				int j;
				unsigned char *org=(unsigned char *)(fc_buff+(i*FILECACHE_TILE)+offset);
				unsigned char *dst=(unsigned char *)buff;
#ifdef DREAMCAST
//				dcache_flush_range((uint32)org,len);
//				dcache_inval_range((uint32)dst,len);
#endif
				for(j=0;j<len;j++)
					*dst++=*org++;
//				memcpy(buff,(void *)(fc_buff+(i*FILECACHE_TILE)+offset),len);
				
				return (void *)(((unsigned)buff)+len);
			}
	}
	return NULL;
}

size_t filecache_read(void *ptr, size_t tam, size_t nmiemb, FILE *flujo)
{
	if (flujo!=fc_file)
		return fread(ptr,tam,nmiemb,flujo);
	int i;
	long len=tam*nmiemb;
	if (fc_pos>=fc_size)
		return 0;
	if (fc_pos+len>=fc_size)
		len=fc_size-fc_pos;

	for(i=0;i<len;)
	{
		if (fc_pos%FILECACHE_TILE)
		{
			int rest=FILECACHE_TILE-(fc_pos%FILECACHE_TILE);
			if (rest>(len-i))
				rest=(len-i);
			filecache_read_block(fc_pos/FILECACHE_TILE);
			ptr=filecache_copy_block(ptr,fc_pos%FILECACHE_TILE,fc_pos/FILECACHE_TILE,rest);
			fc_pos+=rest;
			i+=rest;
		}
		else
		{
			int rest=len-i;
			if (rest>FILECACHE_TILE)
				rest=FILECACHE_TILE;
			filecache_read_block(fc_pos/FILECACHE_TILE);
			ptr=filecache_copy_block(ptr,0,fc_pos/FILECACHE_TILE,rest);
			fc_pos+=rest;
			i+=rest;
		}
	}
	return len/tam;
}

void filecache_init(void)
{
	static int firsttime=0;
	if (!firsttime)
		memset(backup_path,0,64);
	firsttime=1;
	fc_initted=0;
#ifdef DREAMCAST
	unzip_reset_malloc();
#endif
}

void filecache_disable(void)
{
	if (fc_file)
		fclose(fc_file);
	fc_file=NULL;
	memset(backup_path,0,64);
	fc_initted=1;
#ifdef DREAMCAST
	unzip_reset_malloc();
#endif
}

FILE *filecache_open(const char *path, const char *mode)
{
	if (fc_file)
	{
		if (!strncmp((char *)path,backup_path,63))
			return fc_file;
	}

	if (fc_initted)
		return fopen(path,mode);

	FILE *ret=fopen(path,mode);
	if (ret)
	{
		int i;

		if (fc_file)
			fclose(fc_file);
		fc_file=ret;
		strncpy(backup_path,path,63);
		fseek(ret,0,SEEK_END);
		fc_size=ftell(ret);
		fseek(ret,0,SEEK_SET);
		fc_times=0;
		memset(fc_time,0,FILECACHE_N*2);
		memset(fc_used,-1,FILECACHE_N*2);
		fc_pos=0;
		fc_prefetching=1;
		for(i=0;i<FILECACHE_N;i++)
		{
			filecache_read_one_block(i);
			fc_time[i]=FILECACHE_N-i;
		}
		fc_pos=0;
		fc_prefetching=0;
		fc_initted=1;
	}
	return ret;
}

int filecache_close(FILE *flujo)
{
	if (flujo!=fc_file)
		return fclose(flujo);
	return (flujo!=fc_file);
}

int filecache_seek(FILE *flujo, long desplto, int origen)
{
	if (flujo!=fc_file)
		return fseek(flujo,desplto,origen);
	switch(origen)
	{
		case SEEK_END:
			desplto=0x1FFFFFFF;
		case SEEK_SET:
			fc_pos=0;
		default:
			fc_pos+=desplto;
			if (fc_pos>fc_size)
				fc_pos=fc_size;
	}
	return 0;
}

long filecache_tell(FILE *flujo)
{
	if (flujo!=fc_file)
		return ftell(flujo);
	return fc_pos;
}

#endif
