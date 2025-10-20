/* outmode.c
 * - Routines for converting text to binary
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "outmode.h"
#include "tokenize.h"
#include "version.h"
#include "t64.h"
#include "p00.h"
#include "select.h"

#define FALSE 0
#define TRUE 1

#ifdef __EMX__
#define strncasecmp strnicmp
#endif

int outconvert(FILE *, FILE *, int, basic_t);
void make_petscii_name(char petscii_filename[16], const char *filename, char filler);

/* txt2bas
 * - converts a text file into a binary file
 * in:	infile - file name of file to read
 *		force - flag for forcing a BASIC mode (Any for autodetect)
 *		outmode - flag for putting the files in a T64 or P00 container
 * out:	none
 */
void txt2bas(const char *infile, basic_t force, outmode_t outmode)
{
	FILE			*input, *output;
	int				adr;
	basic_t			mode;
	char			text[256], filename[256], *c_p;
	int				morefiles = TRUE;
	int				foundheader, foundextraheader;
	t64header_t		header;
	t64record_t		record;
	p00header_t		p00header;
	unsigned int	totalentries, usedentries, i;
	long			fptr;

	/* First, open input file */
	input = fopen(infile, "rt");
	if (!input) {
		fprintf(stderr, "Unable to open input file: %s\n", infile);
		exit(1);
	}

	/* Secondly, if in T64 mode, open the T64 archive */
	if (T64 == outmode) {
		/* If the T64 file exists, we want to continue adding to it */
		output = fopen("bastext.t64", "r+b");
		if (NULL == output) {
			/* Otherwise, create a new file */
			output = fopen("bastext.t64", "w+b");
			if (NULL == output) {
				fprintf(stderr, "Unable to create output file bastext.t64\n");
				exit(1);
			}

			/* Create standard header */
			memset(&header, 0, sizeof(header));
			const char description[sizeof header.description] = "C64 tape image file " PROGNAME " " PROGVERSION "\x1a";
			memcpy(header.description, description, 32);
			memcpy(header.title, "CREATED BY BASTEXT      ", 24);
			header.version[0]  = 0x00;					/* low */
			header.version[1]  = 0x01;					/* high */
			header.maxfiles[0] = STD_DIRSIZE & 0xFF;	/* low */
			header.maxfiles[1] = STD_DIRSIZE >> 8;		/* high */

			totalentries = STD_DIRSIZE;
			usedentries = 0;
			fwrite(&header, sizeof(header), 1, output);

			/* Fill entries */
			memset(&record, 0, sizeof(record));
			for (i = 0; i < STD_DIRSIZE; i ++) {
				fwrite(&record, sizeof(record), 1, output);
			}
		}
		else {
			/* We opened an old file, now check that it is valid */

			/* Read the T64 header */
			fread(&header, sizeof(header), 1, output);

			/* Check that it is a T64 file */
			if (checkvalidheader(&header, &totalentries, &usedentries,
			    "bastext.t64")) {
				/* It wasn't -> panic */
				exit(1);
			}
		}
	}

	/* Read each available file */
	while (morefiles) {
		/* Set default values */
		adr = 0x0801;						/* default start address */
		mode = (Any == force) ? Basic7
		                      : force;		/* default BASIC mode */

		/* Locate the bastext/tok64 headers */
		foundextraheader = FALSE;
		foundheader = FALSE;
		while (!foundheader && NULL != fgets(text, sizeof(text), input)) {
			/* Remove the trailing newline marker that fgets stuck there */
			text[sizeof(text) - 1] = 0;		/* if buffer was full */
			text[strlen(text) - 1] = 0;		/* overwrite newline */

			/* Check for trailing CR (when reading DOS text files under Unix) */
			if ('\r' == text[strlen(text) - 1]) {
				text[strlen(text) - 1] = 0;
			}

			/* Got a text line, check for tok64 / bastext header */
			if (strncasecmp(text, "start bastext ", 14) == 0) {
				/* Retrieve program start address */
				sscanf(&text[13], "%d", &adr);

				/* If not in force mode, select BASIC dialect from start
				 * address.
				 */
				if (Any == force)	mode = selectbasic(adr);

				/* If the start address was 0x132D, the original file was
				 * a C128 BASIC 7.1 file with the BASIC extension bound to
				 * it. Since we don't have the extension here, we want
				 * instead to start the new file at 0x1C01
				 */
				if (0x132D == adr)	adr = 0x1C01;

				foundextraheader = TRUE;
			}
			else if (strncasecmp(text, "start tok64 ", 12) == 0) {
				/* This is the header that starts the actual BASIC text */
				foundheader = TRUE;

				/* Retrieve the file name */
				strcpy(filename, &text[12]);
			}
			else if (strncasecmp(text, "start tok128 ", 13) == 0) {
				/* This is the header that starts the actual BASIC text */
				foundheader = TRUE;

				/* Retrieve the file name */
				strcpy(filename, &text[13]);

				/* If we didn't find a 'start bastext' header, this was a
				 * standard 0x1C01 C128 BASIC file. Since program starting
				 * here could be BASIC 7.1 too, we set the BASIC version to
				 * 7.1 (superset of 7.0), if it wasn't forced
				 */
				if (!foundextraheader) {
					adr = 0x1C01;
					if (Any == force)	mode = Basic71;
				}
			}
			else if (strncasecmp(text, "start tokx16 ", 13) == 0) {
				/* This is the header that starts the actual BASIC text */
				foundheader = TRUE;

				/* Retrieve the file name */
				strcpy(filename, &text[13]);

				/* This is always a Commander X16 program */
				mode = X16;
			}
		}
		
		if (foundheader) {
			/* A header was found, write a message and open a file
			 * (in T64 mode: create dir entry)
			 */
			fprintf(stderr, "Tokenizing: %s\n", filename);

			if (T64 == outmode) {
				/* Check if the T64 is full */
				if (usedentries >= totalentries) {
					fprintf(stderr, "T64 archive full: bastext.t64\n");
					fclose(input);
					fclose(output);
					exit(1);
				}

				/* Create a T64 file record */
				memset(&record, 0, sizeof(record));
				record.allocflag = ALLOC_NORM;
				record.filetype = 1; /* 0x82? */		/* PRG */
				record.startaddress[0] = adr & 0xFF;	/* low */
				record.startaddress[1] = adr >> 8;		/* high */
				make_petscii_name(record.filename, filename, ' ');

				/* Seek to end of file, and enter start offset into the
				 * file record
				 */
				fseek(output, 0, SEEK_END);
				fptr = ftell(output);
				record.offset[0] = fptr & 0xFF;		/* low */
				record.offset[1] = (fptr >> 8) & 0xFF;
				record.offset[2] = (fptr >> 16) & 0xFF;
				record.offset[3] = fptr >> 24;		/* high */
			}
			else {
				if (P00 == outmode) {
					/* Create output filename; chop at first dot and add
					 * P00 suffix and make all lowercase. We do not do
					 * the full PC64 algorithm here, so no P01, P02,
					 * P03... */
					strcpy(text, filename);
					c_p = strchr(text, '.');
					if (NULL == c_p) {
						c_p = text + strlen(text);
					}
					strcpy(c_p, ".p00"); /* Safe; filename length is max 256 - 12 */
					for (c_p = text; *c_p; ++ c_p) {
						if (*c_p >= 'A' && *c_p <= 'Z') {
							*c_p |= 0x20;
						}
					}
					output = fopen(text, "wb");

					/* Create a P00 file header */
					memset(&p00header, 0, sizeof(p00header));
					strcpy(p00header.description, "C64File");
					make_petscii_name(p00header.filename, filename, '\0');
					fwrite(&p00header, sizeof(p00header), 1, output);
				}
				else {
					/* Write a direct binary file */
					output = fopen(filename, "wb");
				}

				/* Write the start address */
				fputc(adr & 0xFF, output);		/* low */
				fputc(adr >> 8, output);		/* high */
			}

			/* Now convert the file to text */
			adr = outconvert(input, output, adr, mode);

			if (T64 == outmode) {
				/* Finish the T64 record (we now know the ending address)
				 * and write it to the first unused position.
				 */
				record.endaddress[0] = adr & 0xFF;	/* low */
				record.endaddress[1] = adr >> 8;		/* high */

				fseek(output, sizeof(t64header_t) +
				              sizeof(t64record_t) * usedentries, SEEK_SET);
				fwrite(&record, sizeof(t64record_t), 1, output);

				/* Update the T64 header */
				usedentries ++;
				header.numfiles[0] = usedentries & 0xFF;	/* low */
				header.numfiles[1] = usedentries >> 8;		/* high */
				fseek(output, (long) ((t64header_t *) NULL)->numfiles,
				      SEEK_SET);
				fwrite(&header.numfiles, sizeof(header.numfiles),
				       1, output);
			}
			else {
				/* Close output */
				fclose(output);
			}
		}
		else {
			/* If we get here, we have reached EOF */
			morefiles = FALSE;
		}
	}

	/* Close input */
	fclose(input);

	if (T64 == outmode) {
		/* Close T64 */
		fclose(output);
	}
}

/* outconvert
 * - performs the actual conversion
 * in:	input - open file, positioned at start of BASIC text
 * 		output - open file, to write to
 *		adr - address to BASIC start
 *		mode - BASIC version to tokenize
 * out:	last address of file
 */
int outconvert(FILE *input, FILE *output, int adr, basic_t mode)
{
	char		text[512], buf[256];
	int			goon = TRUE;
	int			linelength;
	unsigned	errors = 0;

	/* Read the file until we either find a "stop tok64/tok128" footer,
	 * or get to the end-of-file marker
	 */
	while (goon && NULL != fgets(text, sizeof(text), input)) {
		/* Remove the trailing newline marker that fgets stuck there */
		text[sizeof(text) - 1] = 0;		/* if buffer was full */
		text[strlen(text) - 1] = 0;		/* overwrite newline */

		/* Check for trailing CR (when reading DOS text files under Unix) */
		if ('\r' == text[strlen(text) - 1]) {
			text[strlen(text) - 1] = 0;
		}

		/* Check for trailing backslash (line continuation) */
		while ('\\' == text[strlen(text) - 1]) {
			char	*c_p;

			/* Remove the backslash */
			text[strlen(text) - 1] = 0;

			/* Get next line */
			fgets(buf, sizeof(buf), input);

			/* Remove the trailing newline marker that fgets stuck there */
			buf[sizeof(buf) - 1] = 0;		/* if buffer was full */
			buf[strlen(buf) - 1] = 0;		/* overwrite newline */

			/* Check for trailing CR (when reading DOS text files under Unix) */
			if ('\r' == buf[strlen(buf) - 1]) {
				buf[strlen(buf) - 1] = 0;
			}

			/* Make c_p point to first non-space character */
			c_p = buf;
			while (' ' == *c_p)	c_p ++;

			/* If the combined line isn't too long, combine it */
			if (strlen(text) + strlen(c_p) >= sizeof(text)) {
				fprintf(stderr, "Line too long");
			}
			else {
				strcat(text, buf);
			}
		}

		/* Check if "stop tok64/tok128" marker */
		if (strncasecmp(text, "stop tok", 8) == 0) {
			goon = FALSE;
		}
		else {
			/* Tokenize */
			if (tokenize(text, buf, &linelength, mode)) {
				errors ++;		/* error if nonzero */
			}

			/* Write next-line pointer */
			adr += linelength + 2;
			fputc(adr & 0xFF, output);	/* low */
			fputc(adr >> 8, output);	/* high */

			/* Write line */
			fwrite(&buf, linelength, 1, output);
		}
	}

	/* If we had errors while interpreting the source, say so */
	if (errors) {
		sprintf(text, "63999 REM\"%u errors in tokenization", errors);
		tokenize(text, buf, &linelength, mode);

		/* Write tokenized line to program */
		adr += linelength + 2;
		fputc(adr & 0x7F, output);
		fputc(adr >> 8, output);
		fwrite(&buf, linelength, 1, output);
	}

	/* The program is ended by having a null nextline pointer */
	fputc(0, output);
	fputc(0, output);

	/* adr points to last line pointer, which contains two nulls, so the
	 * last used address is adr+1
	 */
	return adr + 1;
}

/* Remove .prg from filename, copy it to the T64 or P00 record,
 * and confine to uppercase PETSCII.
 *
 * filename must be less than 256 characters long.
 */
void make_petscii_name(char petscii_filename[16], const char *filename, char filler)
{
	char			text[256], *c_p;
	unsigned int	i;

	strcpy(text, filename);
	if (NULL != (c_p = strstr(text, ".prg"))) {
		*c_p = 0;
	} else {
		c_p = text + strlen(text);
	}

	/* Make uppercase, convert _ to spaces, and fill with 'filler'.
	 */
	for (i = 0; i < 16; i ++) {
		if (i >= c_p - text) {
			petscii_filename[i] = filler;
		}
		else if ('_' == petscii_filename[i]) {
		         petscii_filename[i] = ' ';
			petscii_filename[i] = ' ';
		}
		else if (0x60 == (0x60 & text[i])) {
			petscii_filename[i] = text[i] & ~0x20;
		}
		else {
			petscii_filename[i] = text[i];
		}
	}
}
