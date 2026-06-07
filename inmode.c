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

void inconvert(FILE *, FILE *, const char *, int, bool, bool, basic_t);
void from_petscii_name(char title[21], char petscii_filename[16]);

/* bas2txt
 * - converts a binary file into a text file
 * in:	infile - file name of file to read
 *		allfiles - flag whether or not to convert "non-BASIC" files
 *		strict - flag for using strict tok64 compatibility
 *      force - enforced BASIC mode (X16 or VicSuper)
 * out:	none
 */
void bas2txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force)
{
	FILE		*input;
	const char	*title_p;
	char		title_buf[21];
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

	/* Check for P00 file signature - "C64File\0" */
	if (0x3643 == adr)
	{
		char	buf[16];

		if (fgetc(input) != '4' ||
		    fgetc(input) != 'F' ||
		    fgetc(input) != 'i' ||
		    fgetc(input) != 'l' ||
		    fgetc(input) != 'e' ||
		    fgetc(input) != 0)
		{
			/* It wasn't -> panic */
			fprintf(stderr, "Unable to identify file header: %s\n", infile);
			exit(1);
		}

		/* Read the PETSCII file name and point to it */
		memset(buf, 0, sizeof(buf));
		fread(buf, 1, 16, input);
		from_petscii_name(title_buf, buf);
		title_p = title_buf;

		/* Ignore nul and REL metadata */
		fgetc(input);
		fgetc(input);

		/* The file pointer now points to actual data; get the actual start
		 * address  */
		adr = fgetc(input);			/* low byte */
		adr |= fgetc(input) << 8;	/* high byte */
	}

	/* Now convert the file to text */
	inconvert(input, output, title_p, adr, allfiles, strict, force);

	/* Close files */
	fclose(input);
}

void t642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force)
{
	FILE			*input;
	char			title[21];
	t64header_t		header;
	t64record_t		record;
	unsigned int	totalentries, usedentries;
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
	for (unsigned int i = 0; i < usedentries; i ++) {
		/* Seek to the directory entry and read it */
		fseek(input, sizeof(t64header_t) + sizeof(t64record_t) * i, SEEK_SET);
		fread(&record, sizeof(t64record_t), 1, input);

		/* Check filetype */
		if (ALLOC_NORM == record.allocflag) {
			/* This is an allocated entry, with a normal program file in it */

			/* Get the file title */
			from_petscii_name(title, record.filename);

			/* Retrieve the starting address */
			adr = record.startaddress[0] | (record.startaddress[1] << 8);

			/* Position file pointer to the start of data */
			fptr = (record.offset[0]      ) | (record.offset[1] << 8 ) |
			       (record.offset[2] << 16) | (record.offset[3] << 24);
			fseek(input, fptr, SEEK_SET);

			/* Now convert the file to text */
			fprintf(stderr, "Converting: %s\n", title);
			inconvert(input, output, title, adr, allfiles, strict, force);
		}
	}

	/* Close files */
	fclose(input);
}

void d642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force)
{
	FILE			*input, *prgfile;
	uint8_t			sector;
	bam_t			bam;
	dirblock_t		dirblock;
	char			title[21];
	char			tmpfilename[14];
	uint8_t			buf[256];       /* One disk sector */

	/* First, open input file */
	input = opend64(infile, &bam);
	if (!input) {
		fprintf(stderr, "Unable to open input file: %s\n", infile);
		exit(1);
	}

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
				int				adr, fd;
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

				/* inconvert() requires a linear file. Copy the contents to a
				 * temporary file, which we then feed it.
				 *
				 * While tmpfile() is portable (C11), it seems to always write
				 * to /tmp (GNU libc) or the root directory (Microsoft), which
				 * is *bad*.
				 *
				 * For GNU libc and Darwin, we kan use mkstemps(), but for
				 * Microsoft we roll our own code.
				 */
#ifdef _MSC_VER
				for (int j = 0; j < 10; ++ j) {
					snprintf(tmpfilename, sizeof tmpfilename, "bastext%02d.tmp", j);
					fd = _open(tmpfilename,
					           _O_BINARY | _O_CREAT | _O_EXCL | _O_RDWR | _O_TEMPORARY,
					           _S_IREAD | _S_IWRITE);
					if (fd != -1) {
						break;
					}
				}
#else
				strcpy(tmpfilename, "bastextXXXXXX");
				fd = mkstemp(tmpfilename);
#endif
				if (-1 == fd) {
					fprintf(stderr, "Unable to allocate temporary file for %s\n", title);
					continue;
				}
#ifndef _MSC_VER
				/* Make sure we do not leave the file on disk; on Microsoft,
				 * _O_TEMPORARY removes the file on close. On real operating
				 * systems, we call unlink() to remove the file from the
				 * directory even earlier.
				 */
				unlink(tmpfilename);
#endif
				prgfile = fdopen(fd, "w+b");

				/* Make sure loops don't kill us */
				blocks = 664;

				/* End of file marker is track == 0 */
				valid = true;
				while (t && blocks --) {
					/* Read requested sector */
					fseek(input, ts2block(t, s) * 256, SEEK_SET);
					fread(buf, 1, 256, input);

					/* Read links */
					t = buf[0];
					s = buf[1]; /* length of data if t == 0 */

					/* Write to the temporary file */
					fwrite(&buf[2], 1, t ? 254 : s, prgfile);

					/* Check for error conditions */
					if (t && (t == 18 || t > 35 || s > 21)) {
						fprintf(stderr, "File %s contains invalid track/sector links [%d:%d]\n", title, t, s);
						valid = false;
						t = 0;
					}

				}

				if (valid) {
					/* Now rewind the file to read the start address */
					fseek(prgfile, 0, SEEK_SET);
					adr = fgetc(prgfile);		/* low byte */
					adr |= fgetc(prgfile) << 8;	/* high byte */

					/* Now convert the file to text */
					inconvert(prgfile, output, title, adr, allfiles, strict, force);
				}

				/* Closing the temporary files deletes it */
				fclose(prgfile);
			}
		}

		/* Follow the directory links, making sure we stay on the right
		 * track. */
		sector = dirblock.link.s;
		if (dirblock.link.t != 18) sector = 0;
	}

	/* Finish up */
	fclose(input);
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
 * in:	input - open file, positioned at start of BASIC program
 * 		output - open file, to write to
 *		title - program title to print in header
 *		adr - load address of the program
 *		allfiles - flag whether or not to convert "non-BASIC" files
 *      strict - output in strict tok64 compatible mode
 *      force - enforced BASIC mode (X16 or VicSuper)
 * out:	none
 */
void inconvert(FILE *input, FILE *output, const char *title, int adr,
               bool allfiles, bool strict, basic_t force)
{
	int		nextadr;
	bool	valid;
	uint8_t	buf[256];
	basic_t	mode;

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
		valid = true;

		/* Read address to next line */
		nextadr = fgetc(input);			/* low byte */
		nextadr |= fgetc(input) << 8;	/* high byte */

		/* Address to next line is null when the program is ended.
		 * Address to next line must be higher than the current address.
		 * The line cannot be longer than 256 bytes
		 */
		while (nextadr && nextadr > adr && nextadr - adr < 256) {
			/* Read the line into the buffer */
			ptrdiff_t len = nextadr - adr - 2;
			fread(buf, len, 1, input);
			adr = nextadr;

			/* Convert to text */
			if (detokenize(buf, len, output, mode, strict) != 0) {
				valid = false;
			}

			/* Read address to next line */
			nextadr = fgetc(input);			/* low byte */
			nextadr |= fgetc(input) << 8;	/* high byte */
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
