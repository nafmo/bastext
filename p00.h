#pragma once

/* Structure definitions for the P00 container.
 * NB: The structures are written here to be useable without knowing whether
 * the machine's architecture is little- or big-endian (the file format
 * is little-endian).
 */

#pragma pack(push,1)

/* P00 container header, 26 bytes */
typedef struct p00header_s {
	char			description[8];		/* "C64File"+null */
	char			filename[16];		/* Filename (PETSCII), null padded */
	char			nul;				/* null character */
	unsigned char	relsize;			/* REL record size, or null */
} p00header_t;

#pragma pack(pop)
