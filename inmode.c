/* inmode.c
 * Routines for converting binary to text
 */

#include <stdio.h>
#include <string.h>

#include "inmode.h"
#include "tokenize.h"
#include "version.h"
#include "select.h"
#include "t64.h"

#define FALSE 0
#define TRUE 1

void inconvert(FILE *, FILE *, const char *, int, int, int);

/* bas2txt
 * - converts a binary file into a text file
 * in:	infile - file name of file to read
 *		allfiles - flag whether or not to convert "non-BASIC" files
 *		strict - flag for using strict tok64 compatibility
 * out:	none
 */
void bas2txt(const char *infile, FILE *output, int allfiles, int strict)
{
	FILE		*input;
	const char	*title_p;
	int			adr;

	/* First, open input file */
	input = fopen(infile, "rb");
	if (!input) {
		fprintf(stderr, "Unable to open input file: %s\n", infile);
		exit(1);
	}

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

	/* First read the start address */
	adr = fgetc(input);			/* low byte */
	adr |= fgetc(input) << 8;	/* high byte */

	/* Now convert the file to text */
	inconvert(input, output, title_p, adr, allfiles, strict);

	/* Close files */
	fclose(input);
}

void t642txt(const char *infile, FILE *output, int allfiles, int strict)
{
	FILE			*input;
	char			title[21], *c_p;
	t64header_t		header;
	t64record_t		record;
	unsigned int	totalentries, usedentries, i;
	int				adr;
	long			fptr;

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

	/* Cycle through the entries */
	for (i = 0; i < usedentries; i ++) {
		/* Seek to the directory entry and read it */
		fseek(input, sizeof(t64header_t) + sizeof(t64record_t) * i, SEEK_SET);
		fread(&record, sizeof(t64record_t), 1, input);

		/* Check filetype */
		if (ALLOC_NORM == record.allocflag) {
			/* This is an allocated entry, with a normal program file in it */

			/* Get the file title */
			strncpy(title, record.filename, 16);
			title[16] = 0;		/* null terminate */

			while ((char) 32 == title[strlen(title) - 1] ||
			       (char) 160 == title[strlen(title) - 1]) {
				/* Remove trailing spaces */
				title[strlen(title) - 1] = 0;
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

			/* Retrieve the starting address */
			adr = record.startaddress[0] | (record.startaddress[1] << 8);

			/* Position file pointer to the start of data */
			fptr = (record.offset[0]      ) | (record.offset[1] << 8 ) |
			       (record.offset[2] << 16) | (record.offset[3] << 24);
			fseek(input, fptr, SEEK_SET);

			/* Now convert the file to text */
			fprintf(stderr, "Converting: %s\n", title);
			inconvert(input, output, title, adr, allfiles, strict);
		}
	}

	/* Close files */
	fclose(input);
}

/* inconvert
 * - performs the actual conversion
 * in:	input - open file, positioned at start of BASIC program
 * 		output - open file, to write to
 *		title - program title to print in header
 *		allfiles - flag whether or not to convert "non-BASIC" files
 * out:	none
 */
void inconvert(FILE *input, FILE *output, const char *title, int adr,
               int allfiles, int strict)
{
	int		ch, nextadr;
	char	buf[256], text[512];
	basic_t	mode;

	/* Check for valid BASIC file */
	if (allfiles || 0x0401 == adr || 0x0801 == adr || 0x1c01 == adr ||
	    0x4001 == adr || 0x132D == adr) {
		mode = selectbasic(adr);

		/* Print bastext header if start is != 0x0801 and != 0x1C01 */
		if (0x0801 != adr && 0x1C01 != adr) {
			fprintf(output, "\nstart bastext %d", adr);
		}

		/* Print tok64 header */
		if (Basic7 == mode || Basic71 == mode) {
			if (strict) {
				/* tok64 doesn't handle C128 programs, so skip strict mode */
				strict = FALSE;
				fprintf(stderr, "Strict mode ignored for C128 program: %s\n",
				        title);
			}
			fprintf(output, "\nstart tok128 %s\n", title);
		}
		else {
			fprintf(output, "\nstart tok64 %s\n", title);
		}

		/* If this is a combined BASIC 7.1 extension + BASIC text,
		 * skip over the header (0x132D - 0x1C00)
		 */
		if (0x132D == adr) {
			fseek(input, 0x1C01 - 0x132D, SEEK_CUR);
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
		nextadr = fgetc(input);			/* low byte */
		nextadr |= fgetc(input) << 8;	/* high byte */

		/* Address to next line is null when the program is ended.
		 * Address to next line must be higher than the current address.
		 * The line cannot be longer than 256 bytes
		 */
		while (nextadr && nextadr > adr && nextadr - adr < 256) {
			/* Read the line into the buffer */
			fread(buf, nextadr - adr - 2, 1, input);
			adr = nextadr;

			/* Convert to text */
			detokenize(buf, text, mode, strict);

			/* Write to output */
			fputs(text, output);
			fputc('\n', output);

			/* Read address to next line */
			nextadr = fgetc(input);			/* low byte */
			nextadr |= fgetc(input) << 8;	/* high byte */
		}
		
		/* If nextadr != null, then the program was invalid */
		if (nextadr != 0) {
			fprintf(stderr, "Invalid BASIC file: %s\n", title);
			fprintf(output, "63999 REM \"Invalid BASIC input %s\n", title);
		}

		/* Print tok64 footer */
		if (Basic7 == mode || Basic71 == mode) {
			fprintf(output, "stop tok128\n(" PROGNAME ")\n");
		}
		else {
			fprintf(output, "stop tok64\n(" PROGNAME ")\n");
		}
	}
	else {
		fprintf(stderr, "Invalid BASIC start address: %04x (%d)\n", adr, adr);
	}
}