#ifdef DREAMCAST
#include<kos.h>
#endif

#ifdef USE_FILECACHE

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "minimal.h"

#define FILECACHE_SIZE (1920*1024)
#ifndef FILECACHE_TILE
#define FILECACHE_TILE (16*1024)
#endif
#define FILECACHE_SPLIT ((FILECACHE_TILE)/32)
#define FILECACHE_N (FILECACHE_SIZE/FILECACHE_TILE)

#define BACKUP_PATH_SIZE 128
static char backup_path[BACKUP_PATH_SIZE];
#ifndef DREAMCAST
static unsigned char fc_real_buff[FILECACHE_SIZE];
static const unsigned fc_buff=(unsigned)&fc_real_buff;
#else
//static const unsigned fc_buff=0x0820000;
//static const unsigned fc_buff=0xa0820000;
static const unsigned fc_buff=0x20000;
#endif
static short fc_used[FILECACHE_N];
static unsigned short fc_time[FILECACHE_N];
static unsigned char bf_cache[FILECACHE_SPLIT+64+8];
static FILE* fc_file=NULL;
static long fc_times=0;
static long fc_size=0;
static long fc_pos=0;
static int fc_prefetching=0;
static int fc_initted=0;

void unzip_reset_malloc(void);

#ifdef DEBUG_FILECACHE
static unsigned filecache_read_one_block_checksum(int n) {
	unsigned *buf=(unsigned *)(fc_buff+(n*FILECACHE_TILE));
	unsigned i;
	unsigned ret=1245;
	for(i=0;i<FILECACHE_TILE/4;i++)
			ret+=(*buf++)*(i+1);
	return ret;
}
#endif
static int filecache_read_one_block(int n)
{
	int split,i,a=0,at=0x1FFFFFFF;
	if (n>(fc_size/FILECACHE_TILE)) return 1;
	for(i=0;i<FILECACHE_N;i++)
		if (fc_used[i]==n)
		{
			fc_times++;
			fc_time[i]=fc_times;
#ifdef DEBUG_FILECACHE
			printf("filecache_read_one_block(%i)=%p cached\n",n,filecache_read_one_block_checksum(i));
#endif
			return 0;
		}
		else
		if (fc_time[i]<at)
		{
			a=i;
			at=fc_time[i];
		}

	fseek(fc_file,n*FILECACHE_TILE,SEEK_SET);
	for(split=0;split<(FILECACHE_TILE/FILECACHE_SPLIT);split++)
	{
		void *bf=(void *)((((unsigned)&bf_cache[0])&0xffffffe0)+32);
		fread(bf,FILECACHE_SPLIT,1,fc_file);
#ifndef DREAMCAST
		memcpy((void *)(fc_buff+(a*FILECACHE_TILE)+(split*FILECACHE_SPLIT)),bf,FILECACHE_SPLIT);
#else
		spu_memload((uint32)(fc_buff+(a*FILECACHE_TILE)+(split*FILECACHE_SPLIT)),bf,FILECACHE_SPLIT);
#endif
	}

	fc_times++;
	fc_time[a]=fc_times;
	fc_used[a]=n;
#ifdef DEBUG_FILECACHE
	printf("filecache_read_one_block(%i)=%p%s\n",n,filecache_read_one_block_checksum(a),(fc_prefetching?" prefetch":""));
#endif
	return 0;
}

#if 0
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
#else
#define filecache_read_block(N) filecache_read_one_block(N)
#endif

static void *filecache_copy_block_split(unsigned offset, int len)  {
	void *bf=(void *)((((unsigned)&bf_cache[0])&0xffffffe0)+32);
	int diff=0;
	if  (offset&3) {
		diff=offset&3;
	}
#ifdef DEBUG_FILECACHE
	printf("diff=%i ->%i\n",diff,offset&0xfffffffc);
#endif
#ifdef DREAMCAST
	spu_memread(bf,(uint32)(((unsigned)fc_buff)+(offset&0xfffffffc)),len+4);
#else
	memcpy(bf,(void*)(((unsigned)fc_buff)+(offset&0xfffffffc)),len+4);
#endif
	return (void *)(((unsigned)bf)+diff);
}

static void *filecache_copy_block(void *buff, unsigned offset, int n, int len)
{
#ifdef DEBUG_FILECACHE
	printf("filecache_copy_block(buff=%p, offset=%i, n=%i, len=%i\n",buff,offset,n,len);
#endif
	if (buff)
	{
		int i;
		for(i=0;i<FILECACHE_N;i++)
			if (fc_used[i]==n)
			{
				int split;
				for(split=0;split<len;split+=FILECACHE_SPLIT) {
					int l=(len-split>FILECACHE_SPLIT?FILECACHE_SPLIT:len-split);
					void *bf=filecache_copy_block_split((i*FILECACHE_TILE)+offset+split,l);
#ifdef DEBUG_FILECACHE
					printf("memcpy(%p,%p,%i)\n",(void*)(((unsigned)buff)+split),bf,l);
#endif
					memcpy((void*)(((unsigned)buff)+split), bf,l);
				}
#ifdef DEBUG_FILECACHE
				printf("Copy %i to %p from %p (bloque %i)\n",len,buff,fc_buff+(i*FILECACHE_TILE)+offset,i);
#endif
				
				return (void *)(((unsigned)buff)+len);
			}
	}
	return NULL;
}

size_t filecache_read(void *ptr, size_t tam, size_t nmiemb, FILE *flujo)
{
#ifdef DEBUG_FILECACHE
	printf("filecache_read(%p,%i,%i,flujo=%p) fc_file=%p\n",ptr,tam,nmiemb,flujo,fc_file);
#endif
	if (flujo!=fc_file)
		return fread(ptr,tam,nmiemb,flujo);
	int i;
	long len=tam*nmiemb;
#ifdef DEBUG_FILECACHE
	long dbg_pos=ftell(fc_file);
	void *dbg_ptr=ptr;
	long dbg_len=len;
	if (dbg_pos!=fc_pos) {
		printf("fc_pos=%i y deberia ser %i\n",fc_pos,dbg_pos);
		exit(0);
	}
#endif
	if (fc_pos>=fc_size) {
#ifdef DEBUG_FILECACHE
		printf("fc_pos(%i)>=fc_size(%i)\n",fc_pos,fc_size);
#endif
		return 0;
	}
	if (fc_pos+len>=fc_size) {
		len=fc_size-fc_pos;
#ifdef DEBUG_FILECACHE
		printf("len(%i)=fc_size(%i)-fc_pos(%i)\n",len,fc_size,fc_pos);
#endif
	}
#ifdef DEBUG_FILECACHE
	else
		printf("len(%i)=tam(%i)*nmiemb(%i)\n",len,tam,nmiemb);
#endif

	for(i=0;i<len;)
	{
		int rest;
#ifdef DEBUG_FILECACHE
		printf("I=%i, len=%i, fc_pos(%i)%%FILECACHE_TILE=%i\n",i,len,fc_pos,fc_pos%FILECACHE_TILE);
#endif
		if (fc_pos%FILECACHE_TILE)
		{
			rest=FILECACHE_TILE-(fc_pos%FILECACHE_TILE);
			if (rest>(len-i)) {
				rest=(len-i);
#ifdef DEBUG_FILECACHE
				printf("rest(%i)=len(%i)-i(%i)\n",rest,len,i);
#endif
			}
#ifdef DEBUG_FILECACHE
			else
				printf("rest(%i)=FILECACHE_TILE-fc_pos(%i)%%FILECACHE_TILE\n",rest,fc_pos);
#endif
			filecache_read_block(fc_pos/FILECACHE_TILE);
			ptr=filecache_copy_block(ptr,fc_pos%FILECACHE_TILE,fc_pos/FILECACHE_TILE,rest);
		}
		else
		{
			rest=len-i;
			if (rest>FILECACHE_TILE) {
				rest=FILECACHE_TILE;
#ifdef DEBUG_FILECACHE
				printf("rest(%i)=FILECACHE_TILE\n",rest,FILECACHE_TILE);
#endif
			}
#ifdef DEBUG_FILECACHE
			else
				printf("rest(%i)=len(%i)-i(%i)\n",rest,len,i);
#endif
			filecache_read_block(fc_pos/FILECACHE_TILE);
			ptr=filecache_copy_block(ptr,0,fc_pos/FILECACHE_TILE,rest);
		}
		fc_pos+=rest;
		i+=rest;
#ifdef DEBUG_FILECACHE
		printf("fc_pos(%i)+=rest(%i)=%i, i(%i)+=rest=%i\n",fc_pos-rest,rest,fc_pos,i-rest,i);
#endif
	}
#ifdef DEBUG_FILECACHE
	fseek(fc_file,dbg_pos,SEEK_SET);
	void *dbg_mem=calloc(1,dbg_len);
	fread(dbg_mem,1,len,fc_file);
	if (memcmp(dbg_mem,dbg_ptr,dbg_len)) {
		puts("filecache_read ERROR Difiere el contenido");
		exit(0);
	}
	free(dbg_mem);
	fseek(fc_file,dbg_pos+dbg_len,SEEK_SET);
	printf("filecache_read return %i = len(%i)/tam(%i)\n",len/tam,len,tam);
#endif
	return len/tam;
}

void filecache_init(void)
{
	static int firsttime=0;
#ifdef DEBUG_FILECACHE
	printf("filecache_int, firstime=%i\n",firsttime);
#endif
	if (!firsttime)
		memset(backup_path,0,BACKUP_PATH_SIZE);
	firsttime=1;
	fc_initted=0;
#ifdef DREAMCAST
	unzip_reset_malloc();
#endif
#ifdef DEBUG_FILECACHE
	puts("fc_initted=0");
#endif
}

void filecache_disable(void)
{
#ifdef DEBUG_FILECACHE
	puts("filecache_disable");
	exit(0);
#endif
	if (fc_file) {
#ifdef DEBUG_FILECACHE
	puts("Abierto... cerramos");
#endif
		fclose(fc_file);
	}
	fc_file=NULL;
	memset(backup_path,0,BACKUP_PATH_SIZE);
	fc_initted=1;
#ifdef DREAMCAST
	unzip_reset_malloc();
#endif
#ifdef DEBUG_FILECACHE
	puts("fc_initted=1");
#endif
}

FILE *filecache_open(const char *path, const char *mode)
{
#ifdef DEBUG_FILECACHE
	printf("filecache_open('%s','%s')\n",path,mode);
#endif
	if (fc_file)
	{
		if (!strncmp((char *)path,backup_path,BACKUP_PATH_SIZE-1)) {
#ifdef DEBUG_FILECACHE
			printf("Ya abierto: %p\n", fc_file);
			fseek(fc_file,0,SEEK_SET);
#endif
			fc_pos=0;
			return fc_file;
		}
	}

	if (fc_initted) {
#ifdef DEBUG_FILECACHE
		printf("Ya iniciado, fc_initted=%i\n",fc_initted);
#endif
		return fopen(path,mode);
	}

	FILE *ret=fopen(path,mode);
#ifdef DEBUG_FILECACHE
	printf("fopen=%p\n",ret);
#endif
	if (ret)
	{
		int i;

		if (fc_file) {
#ifdef DEBUG_FILECACHE
			printf("Cerramos fc_file=%p\n",fc_file);
#endif
			fclose(fc_file);
		}
		fc_file=ret;
		strncpy(backup_path,path,BACKUP_PATH_SIZE-1);
		fseek(ret,0,SEEK_END);
		fc_size=ftell(ret);
		fseek(ret,0,SEEK_SET);
		fc_times=0;
		memset(fc_time,0,FILECACHE_N*2);
		memset(fc_used,-1,FILECACHE_N*2);
		fc_pos=0;
		fc_prefetching=1;
#ifdef DEBUG_FILECACHE
		printf("Len=%i, fc_times=0, fc_pos=0, fc_prefetching=1\n",fc_size);
#endif
		for(i=0;i<FILECACHE_N;i++)
		{
			filecache_read_one_block(i);
			fc_time[i]=FILECACHE_N-i;
		}
		fc_pos=0;
		fc_prefetching=0;
		fc_initted=1;
#ifdef DEBUG_FILECACHE
		puts("Leido, fc_pos=0, fc_prefetching=0; fc_initted=1");
#endif
	}
#ifdef DEBUG_FILECACHE
	printf("filecache_open=%p\n",ret);
#endif
	return ret;
}

int filecache_close(FILE *flujo)
{
#ifdef DEBUG_FILECACHE
	printf("filechache_close(%p)\n",flujo);
#endif
	if (flujo!=fc_file)
		return fclose(flujo);
	return (flujo!=fc_file);
}

int filecache_seek(FILE *flujo, long desplto, int origen)
{
#ifdef DEBUG_FILECACHE
	printf("filechache_seek(%p, %i, %s)\n",flujo,desplto,(origen==SEEK_END?"SEEK_END":(origen==SEEK_SET?"SEEK_SET":"default")));
#endif
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
#ifdef DEBUG_FILECACHE
	fseek(fc_file,fc_pos,SEEK_SET);
	printf("fc_pos=%i (fc_size=%i)\n",fc_pos,fc_size);
#endif
	return 0;
}

long filecache_tell(FILE *flujo)
{
#ifdef DEBUG_FILECACHE
	printf("filechache_tell(%p)\n",flujo);
#endif
	if (flujo!=fc_file)
		return ftell(flujo);
#ifdef DEBUG_FILECACHE
	printf("ret fc_pos=%i\n",fc_pos);
#endif
	return fc_pos;
}

#endif
