#include "unzip.h"
#include "inflate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef GP2X
#include "gp2x/gp2x_mame.h"
#else
#include "sdl/gp2x_mame.h"
#endif

#ifdef USE_ZIP_CACHE
#define ZIP_CACHE
#endif

#if !defined(GP2X) && defined(USE_FILECACHE)
size_t filecache_read(void *ptr, size_t tam, size_t nmiemb, FILE *flujo);
FILE *filecache_open(const char *path, const char *mode);
int filecache_close(FILE *flujo);
int filecache_seek( FILE *flujo, long desplto, int origen);
long filecache_tell( FILE *flujo);
#else
#define filecache_read(SZ,TM,NM,FL) fread(SZ,TM,NM,FL)
#define filecache_open(PAT,MOD) fopen(PAT,MOD)
#define filecache_close(FL) fclose(FL)
#define filecache_seek(FL,DS,OR) fseek(FL,DS,OR)
#define filecache_tell(FL) ftell(FL)
#endif

#ifndef DREAMCAST
#define unzip_reset_malloc()
#ifdef DEBUG_MEMALLOC_REPORT
void *unzip_malloc(size_t size);
void unzip_free(void *ptr);
#else
#define unzip_free(PTR) if (PTR) gp2x_free(PTR)
#define unzip_malloc(SZ) gp2x_malloc(SZ)
#endif
#else
static unsigned _mmpunt=0x04096000;
static unsigned lastest_punt=0;
static unsigned lastest_size=0;
static __inline__ void *unzip_malloc(size_t NBYTES)
{
	void *ret=(void *)_mmpunt;
	lastest_punt=_mmpunt;
	lastest_size=NBYTES;
	_mmpunt+=(unsigned)NBYTES;
	return ret;
}
void unzip_reset_malloc(void) { _mmpunt=0x04096000; }
#define unzip_free(PTR) \
{ \
	if (((unsigned)(PTR))==lastest_punt) \
	{ \
		_mmpunt-=lastest_size; \
	} \
	else \
	{ if (PTR) gp2x_free(PTR); } \
}
#endif


/* public globals */
int	gUnzipQuiet = 1;		/* flag controls error messages */

#define ERROR_CORRUPT "The zipfile seems to be corrupt, please check it"
#define ERROR_FILESYSTEM "Your filesystem seems to be corrupt, please check it"
#define ERROR_UNSUPPORTED "The format of this zipfile is not supported, please recompress it"

/* Print a error message */
void errormsg(const char* extmsg, const char* usermsg, const char* zipname) {
	/* Output to the user with no internal detail */
	/*if (!gUnzipQuiet)
		printf("Error in zipfile %s\n%s\n", zipname, usermsg);*/
}

/* -------------------------------------------------------------------------
   Unzip support
 ------------------------------------------------------------------------- */

/* Use these to avoid structure padding and byte-ordering problems */
static word read_word (char *buf) {
   unsigned char *ubuf = (unsigned char *) buf;

   return ((word)ubuf[1] << 8) | (word)ubuf[0];
}

/* Use these to avoid structure padding and byte-ordering problems */
static dword read_dword (char *buf) {
   unsigned char *ubuf = (unsigned char *) buf;

   return ((dword)ubuf[3] << 24) | ((dword)ubuf[2] << 16) | ((dword)ubuf[1] << 8) | (dword)ubuf[0];
}

/* Locate end-of-central-dir sig in buffer and return offset
   out:
	*offset offset of cent dir start in buffer
   return:
	==0 not found
	!=0 found, *offset valid
*/
static int ecd_find_sig (char *buffer, int buflen, int *offset)
{
	static char ecdsig[] = { 'P', 'K', 0x05, 0x06 };
	int i;
	for (i=buflen-22; i>=0; i--) {
		if (memcmp(buffer+i, ecdsig, 4) == 0) {
			*offset = i;
			return 1;
		}
	}
	return 0;
}

/* Read ecd data in zip structure
   in:
     zip->fp, zip->length zip file
   out:
     zip->ecd, zip->ecd_length ecd data
*/
static int ecd_read(ZIP* zip) {
	char* buf;
	int buf_length = 1024; /* initial buffer length */

	while (1) {
		int offset;

		if (buf_length > zip->length)
			buf_length = zip->length;

		if (filecache_seek(zip->fp, zip->length - buf_length, SEEK_SET) != 0) {
			return -1;
		}

		/* allocate buffer */
		buf = (char*)unzip_malloc( buf_length );
		if (!buf) {
			return -1;
		}

		if (filecache_read( buf, buf_length, 1, zip->fp ) < 1 /*!=1 FRANXIS 17-01-2005 */) {
			unzip_free(buf);
			return -1;
		}

		if (ecd_find_sig(buf, buf_length, &offset)) {
			zip->ecd_length = buf_length - offset;

			zip->ecd = (char*)unzip_malloc( zip->ecd_length );
			if (!zip->ecd) {
				unzip_free(buf);
				return -1;
			}

			fast_memcpy(zip->ecd, buf + offset, zip->ecd_length);

			unzip_free(buf);
			return 0;
		}

		unzip_free(buf);

		if (buf_length < zip->length) {
			/* double buffer */
			buf_length = 2*buf_length;
		} else {
			return -1;
		}
	}
}

/* offsets in end of central directory structure */
#define ZIPESIG		0x00
#define ZIPEDSK		0x04
#define ZIPECEN		0x06
#define ZIPENUM		0x08
#define ZIPECENN	0x0a
#define ZIPECSZ		0x0c
#define ZIPEOFST	0x10
#define ZIPECOML	0x14
#define ZIPECOM		0x16

/* offsets in central directory entry structure */
#define ZIPCENSIG	0x0
#define ZIPCVER		0x4
#define ZIPCOS		0x5
#define	ZIPCVXT		0x6
#define	ZIPCEXOS	0x7
#define ZIPCFLG		0x8
#define ZIPCMTHD	0xa
#define ZIPCTIM		0xc
#define ZIPCDAT		0xe
#define ZIPCCRC		0x10
#define ZIPCSIZ		0x14
#define ZIPCUNC		0x18
#define ZIPCFNL		0x1c
#define ZIPCXTL		0x1e
#define ZIPCCML		0x20
#define ZIPDSK		0x22
#define ZIPINT		0x24
#define ZIPEXT		0x26
#define ZIPOFST		0x2a
#define ZIPCFN		0x2e

/* offsets in local file header structure */
#define ZIPLOCSIG	0x00
#define ZIPVER		0x04
#define ZIPGENFLG	0x06
#define ZIPMTHD		0x08
#define ZIPTIME		0x0a
#define ZIPDATE		0x0c
#define ZIPCRC		0x0e
#define ZIPSIZE		0x12
#define ZIPUNCMP	0x16
#define ZIPFNLN		0x1a
#define ZIPXTRALN	0x1c
#define ZIPNAME		0x1e

/* Opens a zip stream for reading
   return:
     !=0 success, zip stream
     ==0 error
*/
ZIP* openzip(const char* zipfile) {
	/* allocate */
	ZIP* zip = (ZIP*)unzip_malloc( sizeof(ZIP) );
	if (!zip) {
		return 0;
	}

	/* open */
	zip->fp = filecache_open(zipfile, "rb");
	if (!zip->fp) {
		errormsg ("Opening for reading", ERROR_FILESYSTEM, zipfile);
		unzip_free(zip);
		return 0;
	}

	/* go to end */
	if (filecache_seek(zip->fp, 0L, SEEK_END) != 0) {
		errormsg ("Seeking to end", ERROR_FILESYSTEM, zipfile);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}

	/* get length */
	zip->length = filecache_tell(zip->fp);
	if (zip->length < 0) {
		errormsg ("Get file size", ERROR_FILESYSTEM, zipfile);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}
	if (zip->length == 0) {
		errormsg ("Empty file", ERROR_CORRUPT, zipfile);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}

	/* read ecd data */
	if (ecd_read(zip)!=0) {
		errormsg ("Reading ECD (end of central directory)", ERROR_CORRUPT, zipfile);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}

	/* compile ecd info */
	zip->end_of_cent_dir_sig = read_dword (zip->ecd+ZIPESIG);
	zip->number_of_this_disk = read_word (zip->ecd+ZIPEDSK);
	zip->number_of_disk_start_cent_dir = read_word (zip->ecd+ZIPECEN);
	zip->total_entries_cent_dir_this_disk = read_word (zip->ecd+ZIPENUM);
	zip->total_entries_cent_dir = read_word (zip->ecd+ZIPECENN);
	zip->size_of_cent_dir = read_dword (zip->ecd+ZIPECSZ);
	zip->offset_to_start_of_cent_dir = read_dword (zip->ecd+ZIPEOFST);
	zip->zipfile_comment_length = read_word (zip->ecd+ZIPECOML);
	zip->zipfile_comment = zip->ecd+ZIPECOM;

	/* verify that we can work with this zipfile (no disk spanning allowed) */
	if ((zip->number_of_this_disk != zip->number_of_disk_start_cent_dir) ||
		(zip->total_entries_cent_dir_this_disk != zip->total_entries_cent_dir) ||
		(zip->total_entries_cent_dir < 1)) {
		errormsg("Cannot span disks", ERROR_UNSUPPORTED, zip->zip);
		unzip_free(zip->ecd);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}

	if (filecache_seek(zip->fp, zip->offset_to_start_of_cent_dir, SEEK_SET)!=0) {
		errormsg ("Seeking to central directory", ERROR_CORRUPT, zipfile);
		unzip_free(zip->ecd);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}

	/* read from start of central directory */
	zip->cd = (char*)unzip_malloc( zip->size_of_cent_dir );
	if (!zip->cd) {
		unzip_free(zip->ecd);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}

	if (filecache_read(zip->cd, zip->size_of_cent_dir, 1, zip->fp) < 1 /*!=1 FRANXIS 17-01-2005 */) {
		errormsg ("Reading central directory", ERROR_CORRUPT, zipfile);
		unzip_free(zip->cd);
		unzip_free(zip->ecd);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}

	/* reset ent */
	zip->ent.name = 0;

	/* rewind */
	zip->cd_pos = 0;

	/* file name */
	zip->zip = (char*)unzip_malloc(strlen(zipfile)+1);
	if (!zip->zip) {
		unzip_free(zip->cd);
		unzip_free(zip->ecd);
		filecache_close(zip->fp);
		unzip_free(zip);
		return 0;
	}
	strcpy(zip->zip, zipfile);

	return zip;
}

/* Reads the current entry from a zip stream
   in:
     zip opened zip
   return:
     !=0 success
     ==0 error
*/
struct zipent* readzip(ZIP* zip) {

	/* end of directory */
	if (zip->cd_pos >= zip->size_of_cent_dir)
		return 0;

	/* compile zipent info */
	zip->ent.cent_file_header_sig = read_dword (zip->cd+zip->cd_pos+ZIPCENSIG);
	zip->ent.version_made_by = *(zip->cd+zip->cd_pos+ZIPCVER);
	zip->ent.host_os = *(zip->cd+zip->cd_pos+ZIPCOS);
	zip->ent.version_needed_to_extract = *(zip->cd+zip->cd_pos+ZIPCVXT);
	zip->ent.os_needed_to_extract = *(zip->cd+zip->cd_pos+ZIPCEXOS);
	zip->ent.general_purpose_bit_flag = read_word (zip->cd+zip->cd_pos+ZIPCFLG);
	zip->ent.compression_method = read_word (zip->cd+zip->cd_pos+ZIPCMTHD);
	zip->ent.last_mod_file_time = read_word (zip->cd+zip->cd_pos+ZIPCTIM);
	zip->ent.last_mod_file_date = read_word (zip->cd+zip->cd_pos+ZIPCDAT);
	zip->ent.crc32 = read_dword (zip->cd+zip->cd_pos+ZIPCCRC);
	zip->ent.compressed_size = read_dword (zip->cd+zip->cd_pos+ZIPCSIZ);
	zip->ent.uncompressed_size = read_dword (zip->cd+zip->cd_pos+ZIPCUNC);
	zip->ent.filename_length = read_word (zip->cd+zip->cd_pos+ZIPCFNL);
	zip->ent.extra_field_length = read_word (zip->cd+zip->cd_pos+ZIPCXTL);
	zip->ent.file_comment_length = read_word (zip->cd+zip->cd_pos+ZIPCCML);
	zip->ent.disk_number_start = read_word (zip->cd+zip->cd_pos+ZIPDSK);
	zip->ent.internal_file_attrib = read_word (zip->cd+zip->cd_pos+ZIPINT);
	zip->ent.external_file_attrib = read_dword (zip->cd+zip->cd_pos+ZIPEXT);
	zip->ent.offset_lcl_hdr_frm_frst_disk = read_dword (zip->cd+zip->cd_pos+ZIPOFST);

	/* copy filename */
	unzip_free(zip->ent.name);
	zip->ent.name = (char*)unzip_malloc(zip->ent.filename_length + 1);

	fast_memcpy(zip->ent.name, zip->cd+zip->cd_pos+ZIPCFN, zip->ent.filename_length);
	zip->ent.name[zip->ent.filename_length] = 0;

	/* skip to next entry in central dir */
	zip->cd_pos += ZIPCFN + zip->ent.filename_length + zip->ent.extra_field_length + zip->ent.file_comment_length;

	return &zip->ent;
}

/* Closes a zip stream */
void closezip(ZIP* zip) {
	/* release all */
	unzip_free(zip->ent.name);
	unzip_free(zip->cd);
	unzip_free(zip->ecd);
	/* only if not suspended */
	if (zip->fp)
		filecache_close(zip->fp);
	unzip_free(zip->zip);
	unzip_free(zip);
}

/* Suspend access to a zip file (release file handler)
   in:
      zip opened zip
   note:
     A suspended zip is automatically reopened at first call of
     readuncompressd() or readcompressed() functions
*/
void suspendzip(ZIP* zip) {
	if (zip->fp) {
		filecache_close(zip->fp);
		zip->fp = 0;
	}
}

/* Revive a suspended zip file (reopen file handler)
   in:
     zip suspended zip
   return:
	zip success
	==0 error (zip must be closed with closezip)
*/
static ZIP* revivezip(ZIP* zip) {
	if (!zip->fp) {
		zip->fp = filecache_open(zip->zip, "rb");
		if (!zip->fp) {
			return 0;
		}
	}
	return zip;

}

/* Reset a zip stream to the first entry
   in:
     zip opened zip
   note:
     ZIP file must be opened and not suspended
*/
void rewindzip(ZIP* zip) {
	zip->cd_pos = 0;
}

/* Seek zip->fp to compressed data
   return:
	==0 success
	<0 error
*/
int seekcompresszip(ZIP* zip, struct zipent* ent) {
	char buf[ZIPNAME];
	long offset;

	if (!zip->fp) {
		if (!revivezip(zip))
			return -1;
	}

	if (filecache_seek(zip->fp, ent->offset_lcl_hdr_frm_frst_disk, SEEK_SET)!=0) {
		errormsg ("Seeking to header", ERROR_CORRUPT, zip->zip);
		return -1;
	}

	if (filecache_read(buf, ZIPNAME, 1, zip->fp) < 1 /*!=1 FRANXIS 17-01-2005 */) {
		errormsg ("Reading header", ERROR_CORRUPT, zip->zip);
		return -1;
	}

	{
		word filename_length = read_word (buf+ZIPFNLN);
		word extra_field_length = read_word (buf+ZIPXTRALN);

		/* calculate offset to data and filecache_seek() there */
		offset = ent->offset_lcl_hdr_frm_frst_disk + ZIPNAME + filename_length + extra_field_length;

		if (filecache_seek(zip->fp, offset, SEEK_SET) != 0) {
			errormsg ("Seeking to compressed data", ERROR_CORRUPT, zip->zip);
			return -1;
		}

	}

	return 0;
}

/* Read compressed data
   out:
	data compressed data read
   return:
	==0 success
	<0 error
*/
int readcompresszip(ZIP* zip, struct zipent* ent, char* data) {
	int err = seekcompresszip(zip,ent);
	if (err!=0)
		return err;

	if (filecache_read(data, ent->compressed_size, 1, zip->fp) < 1 /*!=1 FRANXIS 17-01-2005 */) {
		errormsg ("Reading compressed data", ERROR_CORRUPT, zip->zip);
		return -1;
	}

	return 0;
}

/* Read UNcompressed data
   out:
	data UNcompressed data
   return:
	==0 success
	<0 error
*/
int readuncompresszip(ZIP* zip, struct zipent* ent, char* data) {
	if (ent->compression_method == 0x0000) {
		/* file is not compressed, simply stored */

		/* check if size are equal */
		if (ent->compressed_size != ent->uncompressed_size) {
			errormsg("Wrong uncompressed size in store compression", ERROR_CORRUPT,zip->zip);
			return -3;
		}

		return readcompresszip(zip,ent,data);
	} else if (ent->compression_method == 0x0008) {
		/* file is compressed using "Deflate" method */
		if (ent->version_needed_to_extract > 0x14) {
			errormsg("Version too new", ERROR_UNSUPPORTED,zip->zip);
			return -2;
		}

		if (ent->os_needed_to_extract != 0x00) {
			errormsg("OS not supported", ERROR_UNSUPPORTED,zip->zip);
			return -2;
		}

		if (ent->disk_number_start != zip->number_of_this_disk) {
			errormsg("Cannot span disks", ERROR_UNSUPPORTED,zip->zip);
			return -2;
		}

		/* read compressed data */
		if (seekcompresszip(zip,ent)!=0) {
			return -1;
		}

		/* configure inflate */
		if (inflate_file( zip->fp, ent->compressed_size, (unsigned char*)data, ent->uncompressed_size )) {
			errormsg("Inflating compressed data", ERROR_CORRUPT, zip->zip);
			return -3;
		}
		inflate_free();

		return 0;
	} else {
		errormsg("Compression method unsupported", ERROR_UNSUPPORTED, zip->zip);
		return -2;
	}
}

/* -------------------------------------------------------------------------
   Zip cache support
 ------------------------------------------------------------------------- */

/* Use the zip cache */
#ifdef ZIP_CACHE

/* ZIP cache entries */
#define ZIP_CACHE_MAX 2

/* ZIP cache buffer LRU ( Last Recently Used )
     zip_cache_map[0] is the newer
     zip_cache_map[ZIP_CACHE_MAX-1] is the older
*/
static ZIP* zip_cache_map[ZIP_CACHE_MAX];

static ZIP* cache_openzip(const char* zipfile) {
	ZIP* zip;
	unsigned i;

	/* search in the cache buffer */
	for(i=0;i<ZIP_CACHE_MAX;++i) {
		if (zip_cache_map[i] && strcmp(zip_cache_map[i]->zip,zipfile)==0) {
			/* found */
			unsigned j;

			/* reset the zip directory */
			rewindzip( zip_cache_map[i] );

			/* store */
			zip = zip_cache_map[i];

			/* shift */
			for(j=i;j>0;--j)
				zip_cache_map[j] = zip_cache_map[j-1];

			/* set the first entry */
			zip_cache_map[0] = zip;

			return zip_cache_map[0];
		}
	}
	/* not found */

	/* open the zip */
	zip = openzip( zipfile );
	if (!zip)
		return 0;

	/* close the oldest entry */
	if (zip_cache_map[ZIP_CACHE_MAX-1]) {
		/* close last zip */
		closezip(zip_cache_map[ZIP_CACHE_MAX-1]);
		/* reset the entry */
		zip_cache_map[ZIP_CACHE_MAX-1] = 0;
	}

	/* shift */
	for(i=ZIP_CACHE_MAX-1;i>0;--i)
		zip_cache_map[i] = zip_cache_map[i-1];

	/* set the first entry */
	zip_cache_map[0] = zip;

	return zip_cache_map[0];
}

static void cache_closezip(ZIP* zip) {
	unsigned i;

	/* search in the cache buffer */
	for(i=0;i<ZIP_CACHE_MAX;++i) {
		if (zip_cache_map[i]==zip) {
			/* close zip */
			closezip(zip);

			/* reset cache entry */
			zip_cache_map[i] = 0;
			return;

		}
	}
	/* not found */

	/* close zip */
	closezip(zip);
}

#define cache_suspendzip(a) suspendzip(a)

#else

#define cache_openzip(a) openzip(a)
#define cache_closezip(a) closezip(a)
#define cache_suspendzip(a) closezip(a)

#endif

/* -------------------------------------------------------------------------
   Backward MAME compatibility
 ------------------------------------------------------------------------- */

/* Compare two filename
   note:
     don't check directory in zip and ignore case
*/
static int equal_filename(const char* zipfile, const char* file) {
	const char* s1 = file;
	/* start comparison after last / */
	const char* s2 = strrchr(zipfile,'/');
	if (s2)
		++s2;
	else
		s2 = zipfile;
	while (*s1 && toupper(*s1)==toupper(*s2)) {
		++s1;
		++s2;
	}
	return !*s1 && !*s2;
}

/* Pass the path to the zipfile and the name of the file within the zipfile.
   buf will be set to point to the uncompressed image of that zipped file.
   length will be set to the length of the uncompressed data. */
int /* error */ load_zipped_file (const char* zipfile, const char* filename, unsigned char** buf, unsigned int* length) {
	ZIP* zip;
	struct zipent* ent;

	unzip_reset_malloc();
	zip = cache_openzip(zipfile);
	if (!zip)
		return -1;

	ent = readzip(zip);
	while (ent) {
		/* NS981003: support for "load by CRC" */
		char crc[9];

		sprintf(crc,"%08x",ent->crc32);
		if (equal_filename(ent->name, filename) ||
				!strcmp(crc, filename))
		{
			*length = ent->uncompressed_size;
			*buf = (unsigned char*)unzip_malloc( *length );
			if (!*buf) {
				cache_closezip(zip);
				return -1;
			}

			if (readuncompresszip(zip, ent, (char*)*buf)!=0) {
				unzip_free(*buf);
				cache_closezip(zip);
				return -1;
			}

			cache_suspendzip(zip);
			return 0;
		}

		/* next entry */
		ent = readzip(zip);
	}

	cache_suspendzip(zip);
	return -1;
}

/*	Pass the path to the zipfile and the name of the file within the zipfile.
	sum will be set to the CRC-32 of that zipped file. */
/*  The caller can preset sum to the expected checksum to enable "load by CRC" */
int /* error */ checksum_zipped_file (const char *zipfile, const char *filename, unsigned int *length, unsigned int *sum) {
	ZIP* zip;
	struct zipent* ent;

	unzip_reset_malloc();
	zip = cache_openzip(zipfile);
	if (!zip)
		return -1;

	ent = readzip(zip);
	while (ent) {
		if (equal_filename(ent->name, filename))
		{
			*length = ent->uncompressed_size;
			*sum = ent->crc32;
			cache_suspendzip(zip);
			return 0;
		}

		/* next entry */
		ent = readzip(zip);
	}

	cache_suspendzip(zip);

	/* NS981003: support for "load by CRC" */
	zip = cache_openzip(zipfile);
	if (!zip)
		return -1;

	ent = readzip(zip);
	while (ent) {
		if (*sum && ent->crc32 == *sum)
		{
			*length = ent->uncompressed_size;
			*sum = ent->crc32;
			cache_suspendzip(zip);
			return 0;
		}

		/* next entry */
		ent = readzip(zip);
	}

	cache_suspendzip(zip);
	return -1;
}
