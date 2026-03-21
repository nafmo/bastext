#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Structure definitions for D64 format, i.e. a byte-for-byte image of
 * a Commodore 1541 disk.
 */

#pragma pack(push,1)

/* BAM location */
#define BAM_TRACK   18
#define BAM_SECTOR  0

/* Directory location */
#define DIR_TRACK   18
#define DIR_SECTOR  1

/* Sector interleave */
#define D64_INTERLEAVE 3

/* Structure for BAM (18,0) */
typedef struct bam_s
{
	uint8_t t, s;           /* 00-01: Track and sector of next block (18, 1) */
	uint8_t format;         /*    02: DOS version type (0x41) */
	uint8_t unused1;        /*    03: Unusued */
	uint8_t bitmap[140];    /* 04-8F: BAM entries, four bytes per track */
	char    diskname[16];   /* 90-9F: Disk name (PETSCII, padded with 0xA0) */
	uint8_t fill1[2];       /* A0-A1: Filled with 0xA0 */
	uint8_t id[5];          /* A2-A6: 2 char Disk ID, 0xA0, DOS type, but can be anything */
	uint8_t fill2[4];       /* A7-AA: Filled with 0xA0 */
	uint8_t unused2[86];    /* AB-FF: Unused (0x00), used by 40-track extensions or GEOS */
} bam_t;

/* Structure for directory entries */
typedef struct dirent_s
{
	uint8_t filler[2];      /* 00-01: Track and sector of next block (first entry only) */
	uint8_t filetype;       /*    02: File type */
	uint8_t t, s;           /* 03-04: Track and sector of first sector in file */
	char    name[16];       /* 05-14: Name of file (PETSCII, padded with 0xA0) */
	uint8_t st, ss;         /* 15-16: First side-sector block (REL); GEOS: info sector */
	uint8_t rlen;           /*    17: Length for REL file records; GEOS: file structure */
	uint8_t geos_filetype;  /*    18: GEOS: filetype (0x00 for normal C64 file) */
	uint8_t year, month, day;/*19-1B: GEOS: Year - 1900, month, day */
	uint8_t hour, minute;   /* 1C-1D: GEOS: Hour, minute. Also @SAVE temporary storage */
	uint8_t length[2];      /* 1E-1F: File size in sectors */
} dirent_t;

/* Structure for directory blocks (chain starting at 18,1) */
typedef union dirblock_u
{
	dirent_t file[8];       /* 00-FF: Eight directory entries */
	struct
	{
		uint8_t t, s;       /* 00-01: Track and sector of next block */
	} link;
} dirblock_t;

/* File types for filetype field; ORed together */
typedef enum d64_filetype_e {
	/* Bits 0-3: file types */
	D64_DEL = 0x00,     /* Deleted file, with close flag, shows as an unloadable "DEL" file */
	D64_SEQ = 0x01,     /* Sequential file */
	D64_PRG = 0x02,     /* Program file */
	D64_USR = 0x03,     /* User-defined file */
	D64_REL = 0x04,     /* Relative file */
	D64_FTY = 0x07,     /* File type mask */
	/* Bit 6: Lock flag */
	D64_LCK = 0x40,
	/* Bit 7: Close flag; if not set, indicates a splat file */
	D64_CLO = 0x80
} d64_filetype_t;

#pragma pack(pop)

/* Convert combination of track and sector number to a linear block number
 * which is easier to use in D64 files (multiply by 256 for position). */
size_t ts2block(uint8_t track, uint8_t sector);

/* Open a D64 with checking */
FILE *opend64(const char *fname, bam_t *bam_p);
