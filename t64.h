/* t64.h
 */

#ifndef __T64_H
#define __T64_H

/* Structure definitions for the T64 archive format.
 * NB: The structures are written here to be useable without knowing whether
 * the machine's architecture is little- or big-endian (the file format
 * is little-endian).
 */

#pragma pack(1)

/* Default number of entries */
#define STD_DIRSIZE 30

/* T64 archive header, 64 bytes */
typedef struct t64header_s {
	char			description[32];	/* "C64 tape image"+EOF+nulls */
	unsigned char	version[2];			/* $00 / $01 (=$0100) */
	unsigned char	maxfiles[2];		/* word */
	unsigned char	numfiles[2];		/* word */
	unsigned char	reserved[2];
	char			title[24];			/* Title (PETSCII), space padded */
} t64header_t;

/* Values for t64record_t.allocflag */
#define ALLOC_FREE 0
#define ALLOC_NORM 1

/* T64 file record */
typedef struct t64record_s {
	unsigned char	allocflag;			/* 0 = free, 1 = normal, 2.. = others */
	unsigned char	filetype;			/* Filetype (1 = program) / 2ndry address? */
	unsigned char	startaddress[2];	/* Start address of C64 file */
	unsigned char	endaddress[2];		/* Ending address of C64 file */
	unsigned char	reserved1[2];
	unsigned char	offset[4];			/* Start address in T64 */
	unsigned char	reserved2[4];
	char			filename[16];		/* Filename (PETSCII), space padded */
} t64record_t;

/* T64 file layout:
 * 0       t64header_t
 * 64      t64record_t[t64header_t.maxfiles]
 * 64+32*n start of file data
 */

int checkvalidheader(t64header_t *header_p, unsigned int *totalentries_p,
                     unsigned int *usedentries_p, const char *filename);

#endif