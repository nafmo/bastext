/* inmode.c
 * - Routines for converting binary to text
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#else
#include <unistd.h>
#endif
#include "inmode.h"
#include "tokenize.h"
#include "version.h"
#include "select.h"
#include "t64.h"
#include "d64.h"
#include "p00.h"

#define BUFLEN 65535

void inconvert(uint8_t *, size_t, FILE *, const char *, int, bool, bool, basic_t, bool);
void from_petscii_name(char title[21], char petscii_filename[16]);

/* bas2txt
 * - converts a binary file into a text file
 * in:	infile - file name of file to read
 *		allfiles - flag whether or not to convert "non-BASIC" files
 *		strict - flag for using strict tok64 compatibility
 *      force - enforced BASIC mode (X16 or VicSuper)
 *      uppercase - flag for intepreting text strings in uppercase/graphics mode
 * out:	none
 */
void bas2txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force, bool uppercase)
{
	FILE		*input;
	const char	*title_p;
	char		title_buf[21];
	uint8_t		*buf, *p;
	size_t      len;
	int			adr;

	/* First, open input file */
	input = fopen(infile, "rb");
	if (!input) {
		fprintf(stderr, "Unable to open input file: %s\n", infile);
		exit(1);
	}

	/* Read the entire file into a buffer */
	buf = (uint8_t *) malloc(BUFLEN);
	len = fread(buf, 1, BUFLEN, input);

	/* Check for P00 file signature - "C64File\0" */
	if (len > sizeof (p00header_t) + 2 &&
	    0 == ((p00header_t *) buf)->description[7] &&
	    0 == strcmp(((p00header_t *) buf)->description, "C64File")) {
		/* Read the PETSCII file name and point to it */
		from_petscii_name(title_buf, ((p00header_t *) buf)->filename);
		title_p = title_buf;

		/* The start address is after the file header; point to there */
		p = buf + sizeof (p00header_t);
		len -= sizeof (p00header_t);
	}
	else {
		/* Name to print in header is the last part of the file name */
#ifdef __EMX__
		title_p = strrchr(infile, '\\');
#else
		title_p = strrchr(infile, '/');
#endif
		if (title_p) {	/* Found, make pointer point past the slash */
			title_p ++;
		}
		else {	/* Not found, point to the whole file name */
			title_p = infile;
		}

		/* The entire file is data */
		p = buf;
	}

	/* First read the start address */
	adr = *(p ++);			/* low byte */
	adr |= *(p ++) << 8;	/* high byte */

	/* Now convert the file to text */
	inconvert(p, len - 2, output, title_p, adr, allfiles, strict, force, uppercase);

	/* Close files and return memory */
	fclose(input);
	free(buf);
}

void t642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force, bool uppercase)
{
	FILE			*input;
	char			title[21];
	t64header_t		header;
	t64record_t		record;
	unsigned int	totalentries, usedentries;
	int				adr, end;
	long			fptr;
	uint8_t			*buf;

	/* First, open input file */
	input = fopen(infile, "rb");
	if (!input) {
		fprintf(stderr, "Unable to open input file: %s\n", infile);
		exit(1);
	}

	/* Read the T64 header */
	fread(&header, sizeof(header), 1, input);

	/* Check that it is a T64 file */
	if (checkvalidheader(&header, &totalentries, &usedentries, infile)) {
		/* It wasn't -> panic */
		exit(1);
	}

	/* Allocate a temporary buffer to store the program in.
	 *
	 * To avoid having to allocate a new buffer for each entry, we allocate
	 * a global buffer. These machines cannot possibly have BASIC files
	 * larger than 64K, so let's allocate that.
	 */
	buf = malloc(BUFLEN);

	/* Cycle through the entries */
	for (unsigned int i = 0; i < usedentries; i ++) {
		/* Seek to the directory entry and read it */
		fseek(input, sizeof(t64header_t) + sizeof(t64record_t) * i, SEEK_SET);
		fread(&record, sizeof(t64record_t), 1, input);

		/* Check filetype */
		if (ALLOC_NORM == record.allocflag) {
			/* This is an allocated entry, with a normal program file in it */

			/* Get the file title */
			from_petscii_name(title, record.filename);

			/* Retrieve the starting and ending address */
			adr = record.startaddress[0] | (record.startaddress[1] << 8);
			end = record.endaddress[0]   | (record.endaddress[1] << 8);

			/* Position file pointer to the start of data */
			fptr = (record.offset[0]      ) | (record.offset[1] << 8 ) |
			       (record.offset[2] << 16) | (record.offset[3] << 24);
			fseek(input, fptr, SEEK_SET);

			/* Read the entire file into a buffer */
			fread(buf, end - adr, 1, input);

			/* Now convert the file to text */
			fprintf(stderr, "Converting: %s\n", title);
			inconvert(buf, end - adr, output, title, adr, allfiles, strict, force, uppercase);
		}
	}

	/* Close files and return memory */
	fclose(input);
	free(buf);
}

void d642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force, bool uppercase)
{
	FILE			*input;
	uint8_t			sector;
	bam_t			bam;
	dirblock_t		dirblock;
	char			title[21];
	uint8_t			*buf, *end;

	/* First, open input file */
	input = opend64(infile, &bam);
	if (!input) {
		fprintf(stderr, "Unable to open input file: %s\n", infile);
		exit(1);
	}

	/* Allocate a temporary buffer to store the program in.
	 *
	 * To avoid having to allocate a new buffer for each entry, we allocate
	 * a global buffer. These machines cannot possibly have BASIC files
	 * larger than 64K, so let's allocate that.
	 */
	buf = malloc(BUFLEN);

	/* Read the directory; we ignore the BAM completely */
	sector = 1;
	while (sector) {
		/* Load directory block */
		fseek(input, 91392 + sector * 256, SEEK_SET);
		fread(&dirblock, sizeof (dirblock), 1, input);

		/* Each block contains eight directory entries */
		for (size_t i = 0; i < 8; ++ i) {
			if ((dirblock.file[i].filetype & (D64_CLO | D64_FTY)) == (D64_PRG | D64_CLO)) {
				/* This is a valid PRG file entry */
				int				adr;
				size_t			blocks;
				uint8_t			t, s;
				bool			valid;

				/* Get the file title */
				from_petscii_name(title, dirblock.file[i].name);

				/* Check starting position for sanity */
				t = dirblock.file[i].t;
				s = dirblock.file[i].s;
				if (t == 18 || t > 35 || s > 21) {
					fprintf(stderr, "Directory entry for %s is invalid\n", title);
					continue;
				}

				/* inconvert() requires a linear file. Copy the contents to
				 * our temporary buffer, which we then feed it.
				 */

				/* Make sure loops don't kill us */
				blocks = BUFLEN / 254;

				/* End of file marker is track == 0 */
				valid = true;
				end = buf;
				while (t && blocks --) {
					/* Read requested sector */
					fseek(input, ts2block(t, s) * 256, SEEK_SET);
					fread(end, 1, 256, input);

					/* Read links */
					t = end[0];
					s = end[1]; /* length of data if t == 0 */

					/* Shift the buffer two bytes, so that we only have
					 * the program data in it */
					memmove(end, end + 2, 254);
					end += t ? 254 : s;

					/* Check for error conditions */
					if (t && (t == 18 || t > 35 || s > 21)) {
						fprintf(stderr, "File %s contains invalid track/sector links [%d:%d]\n", title, t, s);
						valid = false;
						t = 0;
					}
				}

				if (valid) {
					/* Now read the start address */
					adr = buf[0];		/* low byte */
					adr |= buf[1] << 8;	/* high byte */

					/* Now convert the file to text */
					inconvert(buf + 2, end - buf - 2, output, title, adr, allfiles, strict, force, uppercase);
				}
			}
		}

		/* Follow the directory links, making sure we stay on the right
		 * track. */
		sector = dirblock.link.s;
		if (dirblock.link.t != 18) sector = 0;
	}

	/* Finish up */
	fclose(input);
	free(buf);
}

/* Copy filename from the T64, D64 or P00 record and convert
 * to ASCII.
 *
 * filename is always 16 characters long, but can be padded
 * with nulls, spaces (0x20) or shifted spaces (0xA0).
 *
 * The output buffer must be 21 characters long as we add a
 * ".prg" suffix and a terminating null character */
void from_petscii_name(char title[21], char petscii_filename[16])
{
	char	*c_p;

	memcpy(title, petscii_filename, 16);
	title[16] = 0;		/* null terminate */

	if (title[0]) {
		while ((char) 32 == title[strlen(title) - 1] ||
		       (char) 160 == title[strlen(title) - 1]) {
			/* Remove trailing spaces */
			title[strlen(title) - 1] = 0;
		}
	}

	/* Convert to uppercase ASCII, and change spaces to underscores */
	c_p = title;
	while (*c_p) {
		*c_p &= 0x7F;			/* Strip highbit */
		if (0x60 == (*c_p & 0x60)) {
			*c_p &= ~0x20;		/* Lowercase => uppercase */
		}
		else if (' ' == *c_p) {
			*c_p = '_';
		}
		c_p ++;
	}

	/* Add .prg suffix */
	strcat(title, ".prg");
}

/* inconvert
 * - performs the actual conversion
 * in:	input - pointer to a binary BASIC program
 *      len - length of the program pointer to by input
 * 		output - open file, to write to
 *		title - program title to print in header
 *		adr - load address of the program
 *		allfiles - flag whether or not to convert "non-BASIC" files
 *      strict - output in strict tok64 compatible mode
 *      force - enforced BASIC mode (X16 or VicSuper)
 *      uppercase - flag for intepreting text strings in uppercase/graphics mode
 * out:	none
 */
void inconvert(uint8_t *input, size_t len, FILE *output, const char *title, int adr,
               bool allfiles, bool strict, basic_t force, bool uppercase)
{
	int		nextadr;
	bool	valid;
	uint8_t	*end;
	basic_t	mode;

	end = input + len;

	/* Check for valid BASIC file */
	if (allfiles || 0x0401 == adr || 0x0801 == adr || 0x1001 == adr ||
	    0x1201 == adr || 0x1c01 == adr ||
	    0x4001 == adr || 0x132D == adr) {
		if (force != Any) {
			mode = force;
		}
		else {
			mode = selectbasic(adr);
		}

		/* Print bastext header if start is != 0x0801 and != 0x1C01 */
		if (0x0801 != adr && 0x1C01 != adr) {
			fprintf(output, "\nstart bastext %d " PROGVERSION, adr);
		}

		/* Print tok64 header */
		if (Basic7 == mode || Basic71 == mode) {
			if (strict) {
				/* tok64 doesn't handle C128 programs, so skip strict mode */
				strict = false;
				fprintf(stderr, "Strict mode ignored for C128 program: %s\n",
				        title);
			}
			fprintf(output, "\nstart tok128 %s\n", title);
		}
		else if (Basic4 == mode) {
			/* tok64 doesn't handle BASIC 4 programs, so ignore strict mode */
			strict = false;
			fprintf(output, "\nstart tokpet %s\n", title);
		}
		else if (X16 == mode) {
			/* tok64 doesn't handle X16 programs, so ignore strict mode */
			strict = false;
			fprintf(output, "\nstart tokx16 %s\n", title);
		}
		else {
			fprintf(output, "\nstart tok64 %s\n", title);
		}

		/* If this is a combined BASIC 7.1 extension + BASIC text,
		 * skip over the header (0x132D - 0x1C00)
		 */
		if (0x132D == adr) {
			input += 0x1C01 - 0x132D;
			adr = 0x1C01;
		}

		/* We suppose this is a valid BASIC file, so start reading it
		 * line for line.
		 * Line format is this:
		 *  [0-1]- address to next line
		 *  [2-3]- line number                     \_ sent to
		 *  [4-n]- tokenized line, null terminated /  detokenize
		 */

		/* Read address to next line */
		nextadr = 0xFFFF;
		if (input < end + 2) {
			valid = true;
			nextadr = *(input ++);			/* low byte */
			nextadr |= *(input ++) << 8;	/* high byte */
		}

		/* Address to next line is null when the program is ended.
		 * Address to next line must be higher than the current address.
		 * The line cannot be longer than 256 bytes
		 */
		while (nextadr && nextadr > adr && nextadr - adr < 256) {
			/* Read the line into the buffer */
			ptrdiff_t len = nextadr - adr - 2;
			if (input + len <= end) {
				adr = nextadr;

				/* Convert to text */
				if (detokenize(input, len, output, mode, strict, uppercase) != 0) {
					valid = false;
				}
			}

			/* Read address to next line */
			input += len;
			if (input < end + 2) {
				nextadr = *(input ++);			/* low byte */
				nextadr |= *(input ++) << 8;	/* high byte */
			}
			else {
				nextadr = 0xFFFF;
			}
		}
		
		/* If nextadr != null, then the program was invalid */
		if (nextadr != 0) {
			valid = false;
		}
		if (!valid) {
			fprintf(stderr, "Invalid BASIC file: %s\n", title);
			fprintf(output, "63999 REM \"Invalid BASIC input %s\n", title);
		}

		/* Print tok64 footer; we only check for "stop tok" when reading
		 * the file back, but match the start header above. */
		if (Basic7 == mode || Basic71 == mode) {
			fprintf(output, "stop tok128\n(" PROGNAME " " PROGVERSION ")\n");
		}
		else if (Basic4 == mode) {
			fprintf(output, "stop tokpet\n(" PROGNAME " " PROGVERSION ")\n");
		}
		else if (X16 == mode) {
			fprintf(output, "stop tokx16\n(" PROGNAME " " PROGVERSION ")\n");
		}
		else {
			fprintf(output, "stop tok64\n(" PROGNAME " " PROGVERSION ")\n");
		}
	}
	else {
		fprintf(stderr, "Invalid BASIC start address: %04x (%d)\n", adr, adr);
	}
}
