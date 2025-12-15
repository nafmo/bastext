/* detokenize.c
 * - routines to detokenize C64/C128 BASIC
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "tokenize.h"
#include "tokens.h"

#define FALSE 0
#define TRUE 1

/* The bytestream buffer used in the function (input) is from the line
 * number up to the ending null character. The "next line" pointer is
 * not included
 */

/* detokenize
 * - detokenize a C64/C128 BASIC (in binary) line
 * in:	input_p - pointer to a bytestream to detokenize
 *		output_p - pointer to a string to put results in, MUST BE ALLOCATED
 *      mode - BASIC version to detokenize
 *		strict - flag for using strict tok64 compatibility
 * out:	nonzero on error
 */
int detokenize(const char *input_p, char *output_p, basic_t mode, int strict)
{
	int	quotemode = FALSE;		/* flag for quote mode */
	unsigned short i;			/* loop counter */
	unsigned linenumber;		/* line number */
	int rc = 0;					/* return code */
	int isspecial;				/* flag for special characters */
	const unsigned char *ch_p;	/* pointer moving over input */
	const char *escape_p;		/* pointer to current escape sequence */
	char numeric[4];			/* threedigit numeric escape for strict tok64
								   compatibility */

	ch_p = (const unsigned char*) input_p;

	/* First two bytes is the line number as (low,high) */
	linenumber = (*ch_p) | (*(ch_p + 1)) << 8;
	ch_p += 2;
	
	/* print it to the output string, and move the character pointer beyond */
	output_p += sprintf(output_p, "%u ", linenumber);

	/* Next comes a bytestream of line data, ending in a null character */
	while (*ch_p) {
		/* Point to PETSCII sequence */
		escape_p = petscii[*ch_p];
		if (strict && nontok64compatible(*ch_p)) {
			/* Maintain tok64 compatibility */
			sprintf(numeric, "%03d", (int) *ch_p);
			escape_p = numeric;
		}

		/* Process token */
		if (quotemode) {		/* quoted string? */
			/* Convert from PETSCII to ASCII,
			 * and write repetitions as a multiple of the character.
			 * Repetitions of non-special characters is only written if
			 * there are three or more repetitions.
			 * Repetitions of * is not written ({**n} is not parsed correctly)
			 * Repetitions of " is not written (quotemode on/off)
			 */
			if (34 == *ch_p) {		/* quote */
				*(output_p ++) = '\"';
				quotemode = FALSE;	/* go out of quotemode */
			} /* if */
			else if (42 == *ch_p) {	/* asterisk */
				*(output_p ++) = '*';
			} /* else */
			else {
				/* Check for special token (escape is multibyte) */
				if (escape_p[1] == 0) {
					isspecial = FALSE;
				} /* if */
				else {
					isspecial = TRUE;
				} /* else */

				/* Check repetition if:
				 *  current and next character match
				 *  AND (at least) one of the following:
				 *    <this is a special character OR space>
				 *    OR current and third character match
				 *  AND (at least) one of the following:
				 *    we are not in tok64 strict compatibility mode
				 *    OR the character is space
				 *    OR the escape code is not a single character
				 */
				if (*ch_p == ch_p[1] &&
				    ((isspecial || 32 == *ch_p) ||
				     *ch_p == ch_p[2]) &&
				    (!strict || 32 == *ch_p || strlen(escape_p) > 1)) {
					/* Count repetitions */
					i = 2;
					while (ch_p[i] == *ch_p) i ++;

					/* We know the repetition number, now print it */
					if (32 == *ch_p) {	/* space */
						output_p += sprintf(output_p, "{space*%hd}", i);
					} /* if */
					else {
						output_p += sprintf(output_p, "{%s*%hd}", escape_p, i);
					} /* else */

					ch_p += i - 1;	/* point to last repetition */
				} /* if */
				else {	/* not repetition */
					if (isspecial) {
						output_p += sprintf(output_p, "{%s}", escape_p);
					} /* if */
					else {	/* normal character */
						*(output_p ++) = *escape_p;
					} /* else */
				} /* else */
			} /* else */
		} /* if */
		else {					/* command mode */
			if (*ch_p >= 128 && *ch_p <= 254) {	/* Probable BASIC command */
				if ((unsigned char) *ch_p <= 203) {
					/* C64 BASIC 2.0 */
					output_p += sprintf(output_p, "%s",
					                    c64tokens[*ch_p - 128]);
				} /* if */
				else if (*ch_p == 0xCE &&
				         (*(ch_p + 1) >= 2 && *(ch_p + 1) <= 0xA) &&
				         (Basic7 == mode || Basic71 == mode)) {
					/* C128 BASIC 7.0 CE prefix */
					ch_p ++;
					output_p += sprintf(output_p, "%s",
					                    c128CEtokens[*ch_p]);
				//*****************************************************//
				} /* else */
				else if (*ch_p == 0xCE &&
				         ((*(ch_p + 1) >= 0x80 && *(ch_p + 1) <= 0xC1) ||
				          (*(ch_p + 1) >= 0xD0 && *(ch_p + 1) <= 0xDD)) &&
				          (X16 == mode)) {
					/* X16 CE prefix */
					ch_p ++;
					output_p += sprintf(output_p, "%s",
					                    x16tokens[(*ch_p) - 0x80]);
				//*****************************************************//
				} /* else */
				else if (*ch_p == 0xFE && *(ch_p + 1) >= 2 &&
				         ((*(ch_p + 1) <= 0x26 && Basic7 == mode) ||
				          (*(ch_p + 1) <= 0x37 && Basic71 == mode))) {
					/* C128 BASIC 7.0/7.1 FE prefix */
					ch_p ++;
					output_p += sprintf(output_p, "%s",
					                    c128FEtokens[*ch_p]);
				} /* else */
				else if (Basic35 == mode || Basic7 == mode || Basic71 == mode) {
					/* Commodore 16/Plus4 BASIC 3.5 or C128 BASIC 7.0 */
					output_p += sprintf(output_p, "%s",
					                    c128tokens[*ch_p - 204]);
				} /* else */
				else if (*ch_p <= 221 && VicSuper == mode) {
					/* VIC-20 Super Expander */
					output_p += sprintf(output_p, "%s",
					                    supertokens[*ch_p - 204]);
				}
				else if (Graphics52 == mode) {
					/* C64 Graphics52 */
					output_p += sprintf(output_p, "%s",
					                    graphics52tokens[*ch_p - 204]);
				} /* else */
				else if (*ch_p <= 232 && TFC3 == mode) {
					/* C64 TFC3 */
					output_p += sprintf(output_p, "%s",
					                    tfc3tokens[*ch_p - 204]);
				} /* else */
				else {
					/* Errorneous token */
					output_p += sprintf(output_p, "{%d}", *ch_p);
				}
				
			} /* if */
			else {				/* text */
				/* PETSCII text in BASIC:
				 * The only possible case of text is unshifted. To increase
				 * readability, this is written as lowercase ASCII, whereas
				 * keywords are written as uppercase.
				 * There can also be special characters (32-64), they are
				 * printed as-is.
				 */
				if ((*ch_p >= 32 && *ch_p <= 64) ||	/* ' ' - '@', */
				    91 == *ch_p || 93 == *ch_p) {		/* '[', ']' */
					*(output_p ++) = *ch_p;
					if (34 == *ch_p) {
						quotemode = TRUE;		/* go to quotemode */
					} /* if */
				} /* if */
				else if (*ch_p >= 65 && *ch_p <= 90) {	/* 'A' - 'Z' */
					*(output_p ++) = tolower(*ch_p);
				} /* else */
				else {	/* Possibly illegal character, write petscii escape */
					output_p += sprintf(output_p, "{%s}", escape_p);
				} /* else */
			} /* else */
		} /* else */

		ch_p ++;				/* next character */
	} /* while */

	*output_p = 0;

	return rc;
}
