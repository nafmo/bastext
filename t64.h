#pragma once
#include <stdint.h>

/* Structure definitions for the T64 archive format.
 * NB: The structures are written here to be useable without knowing whether
 * the machine's architecture is little- or big-endian (the file format
 * is little-endian).
 */

#pragma pack(push,1)

/* Default number of entries */
#define STD_DIRSIZE 30

/* T64 archive header, 64 bytes */
typedef struct t64header_s {
	char			description[32];	/* "C64 tape image"+EOF+nulls */
	uint8_t			version[2];			/* $00 / $01 (=$0100) */
	uint8_t			maxfiles[2];		/* word */
	uint8_t			numfiles[2];		/* word */
	uint8_t			reserved[2];
	char			title[24];			/* Title (PETSCII), space padded */
} t64header_t;

/* Values for t64record_t.allocflag */
#define ALLOC_FREE 0
#define ALLOC_NORM 1

/* T64 file record */
typedef struct t64record_s {
	uint8_t			allocflag;			/* 0 = free, 1 = normal, 2.. = others */
	uint8_t			filetype;			/* Filetype (1 = program) / 2ndry address? */
	uint8_t			startaddress[2];	/* Start address of C64 file */
	uint8_t			endaddress[2];		/* Ending address of C64 file */
	uint8_t			reserved1[2];
	uint8_t			offset[4];			/* Start address in T64 */
	uint8_t			reserved2[4];
	char			filename[16];		/* Filename (PETSCII), space padded */
} t64record_t;

#pragma pack(pop)

/* T64 file layout:
 * 0       t64header_t
 * 64      t64record_t[t64header_t.maxfiles]
 * 64+32*n start of file data
 */

int checkvalidheader(t64header_t *header_p, unsigned int *totalentries_p,
                     unsigned int *usedentries_p, const char *filename);
