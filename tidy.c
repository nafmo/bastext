/* tidy.c
 * - a small program that makes the groff outputted doc file not contain
 *   rubout sequences for bold and underline styles
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE	*infile, *outfile;
	int		ch, nch;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
		return 1;
	}

	infile = fopen(argv[1], "rt");
	outfile = fopen(argv[2], "wt");

	if (!infile || !outfile) {
		fprintf(stderr, "%s: error opening files", argv[0]);
		return 1;
	}

	ch = 0;
	while (EOF != (nch = fgetc(infile))) {
		if (8 == nch) {	/* ^H = rubout, kill this and the previous character */
			nch = fgetc(infile);
		}
		else {
			if (ch)	fputc(ch, outfile);
		}
		ch = nch;
	}
	fputc(ch, outfile);

	fclose(infile);
	fclose(outfile);
}