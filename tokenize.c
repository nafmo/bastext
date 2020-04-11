/* tokenize.c
 * - routines to tokenize C64/C128 BASIC
 * $Id$
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "tokenize.h"
#include "tokens.h"

#define FALSE 0
#define TRUE 1

#ifdef __EMX__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

/* The bytestream buffer used in the function (output) is from the line
 * number up to the ending null character. The "next line" pointer is
 * not included
 */

/* tokenize
 * - tokenizes a C64/C128 BASIC (in tok64 pseudocode) line
 * in:	input_p - pointer to string to tokenize
 *		output_p - pointer to bytestream to put results in, MUST BE ALLOCATED
 *		length_p - pointer to integer to write length counter to
 *      mode - BASIC version to tokenize
 * out:	nonzero on error
 */
int tokenize(const char *input_p, char *output_p, int *length_p, basic_t mode)
{
	int	quotemode = FALSE;		/* flag for quote mode */
	unsigned short i;			/* loop counter */
	unsigned linenumber;		/* line number */
	int inputleft = strlen(input_p);	/* amount left of line to tokenize */
	int tokenlen;				/* length of current token */
	int match;					/* match found flag */
	int notokenize = FALSE;		/* REM/DATA no tokenize flag */
	int rc = 0;					/* return code */
	char buf[16];				/* buffer for special character match */
	char *start_p = output_p;	/* pointer to input */

	/* Skip any initial whitespace */
	while (' ' == *input_p || '\t' == *input_p)	input_p ++;

	/* Get the line number */
	linenumber = 0;
	while (isdigit(*input_p)) {	/* line number consits of numerals */
		linenumber = linenumber * 10 + (*input_p - '0');
		input_p ++;
	} /* while */

	if (linenumber >= 64000) {
		rc = 1;
		fprintf(stderr, "* Illegal line number: %u\n", linenumber);
	} /* if */

	/* Insert line number in byte stream */
	*(output_p ++) = linenumber & 255;	/* low */
	*(output_p ++) = linenumber >> 8;	/* high */
	
	/* Kill off any extraneous spaces */
	while (' ' == *input_p) input_p ++;

	/* Now process the rest of the line */
	while (*input_p) {			/* while string isn't ended */
		if ('{' == *input_p) {	/* special character */
			/* copy special character name to buffer
			 * character name ends with '}' or '*'
			 */
			i = 0;				/* buffer position counter */
			input_p ++;			/* position at first character of name */
			while (i < 16 && *input_p != '*' && *input_p != '}' && *input_p) {
				/* terminate loop on:
				 *  . buffer size overrun (error)
				 *  . '*' in input stream
				 *  . '}' in input stream
				 *  . null in input stream (error)
				 *
				 * else: copy from character name to buffer
				 */
				buf[i ++] = *(input_p ++);
			} /* while */
			buf[i] = 0;	/* terminate with null */
			/* Check for error condition */
			if (*input_p != '*' && *input_p != '}') {
				rc = 1;
				fprintf(stderr, "* Special character sequence incorrect: '{%s'"
				                " at line %u\n",
				        buf, linenumber);
			} /* if */
			else {
				/* It seems to be ok, so look it up */
				match = FALSE;

				/* Threedigit numeric? */
				if (strlen(buf) == 3 &&
				    isdigit(buf[0]) && isdigit(buf[1]) && isdigit(buf[2])) {
					sscanf(buf, "%d", &match);
				}

				/* Look it up in the PETSCII table */
				for (i = 1; i <= 255 && !match; i ++) {
					if ((i & 0x7f) >= 0x41 && (i & 0x7f) <= 0x5A) {
						/* Upper-/lowercase PETSCII must be matched with
						 * case also (otherwise 'e' would match 'E')
						 */
						if (0 == strcmp(petscii[i], buf))
							match = i;		/* match */
					} /* if */
					else {
						if (0 == strcasecmp(petscii[i], buf))
							match = i;		/* match */
					} /* else */
				} /* for */

				/* Special condition: space (32) can be repeated, and
				 * is then called 'space', which is not in the petscii
				 * table
				 */

				if (!match && 0 == strcasecmp("space", buf)) {
					match = ' ';
				} /* if */

				/* Now check whether or not we got a match */
				if (match) {
					/* We have found which PETSCII character was meant,
					 * now we check whether or not we wanted more than one
					 * character of this kind ("{char*n}")
					 */
					i = 1;		/* one copy wanted */
					if ('*' == *input_p) { /* multiple copies wanted */
						input_p ++;	/* adjust to point to counter */
						i = 0;
						while (isdigit(*input_p)) {
							i = i * 10 + (*input_p - '0');	/* count */
							input_p ++;
						} /* while */

						/* Check for error condition */
						if ('}' != *input_p || i == 0 || i > 255) {
							rc = 1;
							i = 0;
							fprintf(stderr, "* Illegal character count at line "
							                "%u\n",
							        linenumber);
						} /* if */
					} /* if */

					if (i > 0) {
						/* Copy the wanted number of characters */
						while (i) {
							*(output_p ++) = match;
							i --;
						} /* while */
						
						/* Input should now point to the } end delimeter,
						 * skip this
						 */
						input_p ++;
					} /* if */
				} /* if */
				else {
					rc = 1;
					fprintf(stderr, "* Illegal special character: {%s} at "
					                "line %u\n",
					        buf, linenumber);
				} /* else */
			} /* else */
		} /* if */
		else if (!quotemode) {	/* check for token */
			match = FALSE;

			/* Skip tokenization attempt if:
			 *  . No tokenization flag is set
			 *  . Input string starts with numeral or space
			 *    (no tokens starts with numerals or spaces)
			 */
			if (notokenize || ' ' == *input_p || isdigit(*input_p))
				goto skiptokenize;		/* Looks better than nested if */

			/* C64 BASIC */
			for (i = 0; i <= 75 && !match; i ++) {
				tokenlen = strlen(c64tokens[i]);	/* -=TODO:=- table lookup */
				if (inputleft >= tokenlen &&
				    0 == strncasecmp(input_p, c64tokens[i], tokenlen)) {
					/* token match found */
					match = TRUE;
					(*output_p ++) = i + 128;	/* write token */
					input_p += tokenlen;		/* skip token */
					inputleft -= tokenlen;

					if (15 == i || 3 == i) {	/* REM & DATA */
						notokenize = TRUE;
					}
				} /* if */
			} /* for */

			/* C128 BASIC 7.0/7.1 */
			if (!match && (Basic7 == mode || Basic71 == mode)) {
				for (i = 2; i <= ((mode == Basic7) ? 38 : 55) && !match;
				     i ++) {
					tokenlen = strlen(c128FEtokens[i]);	/* as above */
					if (tokenlen && inputleft >= tokenlen &&
					    0 == strncasecmp(input_p, c128FEtokens[i], tokenlen)) {
						/* token match found */
						match = TRUE;
						(*output_p ++) = 0xFE;		/* token escape */
						(*output_p ++) = i;			/* write token */
						input_p += tokenlen;		/* skip token */
						inputleft -= tokenlen;
					} /* if */
				} /* for */

				if (match) goto skipover;	/* nicer than nested ifs */

				for (i = 2; i <= 9 && !match; i ++) {
					tokenlen = strlen(c128CEtokens[i]);	/* as above */
					if (tokenlen && inputleft >= tokenlen &&
					    0 == strncasecmp(input_p, c128CEtokens[i], tokenlen)) {
						/* token match found */
						match = TRUE;
						(*output_p ++) = 0xCE;		/* token escape */
						(*output_p ++) = i;			/* write token */
						input_p += tokenlen;		/* skip token */
						inputleft -= tokenlen;
					} /* if */
				} /* for */
			skipover: ;
			} /* if */

			if (!match && (Basic7 == mode || Basic71 == mode ||
			               Basic35 == mode)) {
				for (i = 0; i <= 49 && !match; i ++) {
					if (0xCE == i && Basic35 != mode) i ++;
						/* skip prefix 0xCE in BASIC 7.0/7.1 */
					tokenlen = strlen(c128tokens[i]);	/* as above */
					if (tokenlen && inputleft >= tokenlen &&
					    0 == strncasecmp(input_p, c128tokens[i], tokenlen)) {
						/* token match found */
						match = TRUE;
						(*output_p ++) = i + 204;	/* write token */
						input_p += tokenlen;		/* skip token */
						inputleft -= tokenlen;
					} /* if */
				} /* for */
			} /* if */

			/* TFC3 */
			if (!match && TFC3 == mode) {
				for (i = 0; i <= 28 && !match; i ++) {
					tokenlen = strlen(tfc3tokens[i]);	/* as above */
					if (inputleft >= tokenlen &&
					    0 == strncasecmp(input_p, tfc3tokens[i], tokenlen)) {
						/* token match found */
						match = TRUE;
						(*output_p ++) = i + 204;	/* write token */
						input_p += tokenlen;		/* skip token */
						inputleft -= tokenlen;
					} /* if */
				} /* for */
			} /* if */

			/* Graphics52 */
			if (!match && Graphics52 == mode) {
				for (i = 0; i <= 50 && !match; i ++) {
					tokenlen = strlen(graphics52tokens[i]);	/* as above */
					if (inputleft >= tokenlen &&
					    0 == strncasecmp(input_p, graphics52tokens[i],
					                     tokenlen)) {
						/* token match found */
						match = TRUE;
						(*output_p ++) = i + 204;	/* write token */
						input_p += tokenlen;		/* skip token */
						inputleft -= tokenlen;
					} /* if */
				} /* for */
			} /* if */

			/* PET BASIC 4.0/C64 BASIC 4.0 extension */
			if (!match && Basic4 == mode) {
				for (i = 0; i <= 23 && !match; i ++) {
					tokenlen = strlen(basic4tokens[i]); /* as above */
					if (inputleft >= tokenlen &&
					    0 == strncasecmp(input_p, basic4tokens[i], tokenlen)) {
						/* token match found */
						match = TRUE;
						(*output_p ++) = i + 204;	/* write token */
						input_p += tokenlen;		/* skip token */
						inputleft -= tokenlen;
					} /* if */
				} /* for */
			} /* if */

			/* VIC Super Extender BASIC extension */
			if (!match && VicSuper == mode) {
				for (i = 0; i <= 17 && !match; i ++) {
					tokenlen = strlen(supertokens[i]); /* as above */
					if (inputleft >= tokenlen &&
					    0 == strncasecmp(input_p, supertokens[i], tokenlen)) {
						/* token match found */
						match = TRUE;
						(*output_p ++) = i + 204;	/* write token */
						input_p += tokenlen;		/* skip token */
						inputleft -= tokenlen;
					} /* if */
				} /* for */
			} /* if */

skiptokenize:

			if (!match) {
				/* there was no match on a token, just convert to
				 * petscii and write out.
				 * Since everything other than lowercase letters and
				 * special characters should have been catched here,
				 * everything should be written as such.
				 * Other characters are reported as errors.
				 */
				if ((*input_p >= 32 && *input_p <= 91) ||
				    *input_p == 93) {
					/* these need not to be converted
					 * (uppercase ASCII => lowercase PETSCII)
					 */
					*(output_p ++) = *input_p;
					if ('\"' == *input_p) {
						quotemode = !quotemode;		/* invert quotemode */
					} /* if */
					input_p ++;
					inputleft --;
				} /* if */
				else if (*input_p >= 96 && *input_p <= 122) {
					/* lowercase ASCII, convert to lowercase PETSCII) */
					*(output_p ++) = *input_p - 32;
					input_p ++;
					inputleft --;
				} /* if */
				else {			/* illegal character */
					fprintf(stderr, "* Illegal character in input (nonquoted): "
					                "%c (%hu) at line %u\n",
					        *input_p, (unsigned short) *input_p, linenumber);
					input_p ++;
					rc = 1;
				} /* else */
			} /* if */
		} /* else */
		else if (quotemode) {	/* non-special character quoted */
			/* Map special characters (32-64) on themselves
			 *     uppercase ASCII    (65-90) on uppercase PETSCII (193-228)
			 *     lowercase ASCII   (97-122) on lowercase PETSCII  (65-90)
			 *     other specials  (91,93,94) on themselves
			 * other characters should not be there
			 */
			if ((*input_p >= 32 && *input_p <= 64) ||
			    (91 == *input_p || 93 == *input_p || 94 == *input_p)) {
			    *(output_p ++) = *input_p;
				if ('\"' == *input_p) {
					quotemode = !quotemode;		/* invert quotemode */
				} /* if */
				input_p ++;
				inputleft --;
			} /* if */
			else if (*input_p >= 65 && *input_p <= 90) {
				*(output_p ++) = *input_p | 128;
				input_p ++;
				inputleft --;
			} /* if */
			else if (*input_p >= 97 && *input_p <= 122) {
				*(output_p ++) = *input_p & (~32);
				input_p ++;
				inputleft --;
			} /* if */
			else {
				/* Unknown character */
				fprintf(stderr, "* Illegal character in input (quoted): "
				                "%c (%hu) at line %u\n",
				        *input_p, (unsigned short) *input_p, linenumber);
				input_p ++;
				rc = 1;
			} /* else */
		} /* else */
	} /* while */
	
	*output_p = 0;				/* end bytestream with a 0 */
	*length_p = output_p - start_p + 1;	/* tokenized stream length */
	return rc;					/* return errorcode */
}
