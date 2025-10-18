/* main.c
 * - main routines for bastext
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __EMX__
# include <getopt.h>
#else
# include <unistd.h>
#endif

#include "inmode.h"
#include "outmode.h"
#include "tokenize.h"

#define TRUE 1
#define FALSE 0

typedef enum runmode_e { None, In, Out } runmode_t;

#ifdef __EMX__
# define SWITCH "/"
#else
# define SWITCH "-"
#endif

/* main
 * - main routine
 * evaluates arguments and call the appropriate routines
 */
int main(int argc, char *argv[])
{
	int			option, i;
	int			allfiles = FALSE;
	int			t64mode = FALSE;
	int			strict = FALSE;
	runmode_t	mode = None;
	basic_t		force = Any;
	char		*outfile = "-";
	FILE		*output;

#ifdef __EMX__
	/* OS/2 uses '/' as switch character */
	optswchar = SWITCH;
#endif

	/* Recognized options:
	 *  i (in)   - convert from binary to text
	 *  o (out)  - convert from text to binary
	 *  t (t64)  - T64 mode
	 *  2 (2.0)  - force BASIC 2.0        -\
	 *  3 (TFC3) - force TFC3 BASIC         \
	 *  5 (G52)  - force Graphics52 BASIC    >- out mode only,
	 *  7 (7.0)  - force BASIC 7.0          /   if not specified, looks at
	 *  1 (7.1)  - force BASIC 7.1        -/    "start bastext" header
	 *  x (x16)  - force X16 BASIC (r48)
	 *  a (all)  - convert all programs, not only those with recognized start
	 *             address (0401/0801/1001/1201/132D/1C01/4001)
	 *  s (strict)-strict tok64 encoding
	 *  d (dest) - gives destination filename (followed by filename)
	 *  h (help) - print help page
	 *  ? (help)
	 */
	while (-1 != (option = getopt(argc, argv, "iot23571xasd:h?"))) {
		switch (option) {
			case 'i':
				mode = In;
				break;

			case 'o':
				mode = Out;
				break;

			case 't':
				t64mode = TRUE;
				break;

			case '2':
				force = Basic2;
				break;

			case '3':
				force = TFC3;
				break;

			case '5':
				force = Graphics52;
				break;

			case '7':
				force = Basic7;
				break;

			case '1':
				force = Basic71;
				break;

			case 'x':
				force = X16;
				break;

			case 'a':
				allfiles = TRUE;
				break;

			case 's':
				strict = TRUE;
				break;

			case 'd':
				outfile = optarg;
				break;

			case 'h':
			case '?':
			case ':':
				fprintf(stderr, "Usage: %s " SWITCH "i|" SWITCH "o|" SWITCH "h [modifiers] infile(s) outfile\n"
				                "\n Mode (one of these required):\n"
				                "  " SWITCH "i\tInput mode (binary to text)\n"
				                "  " SWITCH "o\tOutput mode (text to binary)\n"
				                "  " SWITCH "h\tPrint help page\n"
				                "\n General modfiers:\n"
				                "  " SWITCH "t\tT64 mode (in: reads from specified T64 archive(s)\n"
				                "    \t          out: creates/appends to bastext.t64)\n"
				                "\n Input mode modfiers:\n"
				                "  " SWITCH "a\tConvert all, not just recognized start addresses\n"
				                "    \t (0401/0801/1001/1201/132D/1C01/4001)\n"
				                "  " SWITCH "s\tStrict tok64 compatibility\n"
				                "  " SWITCH "d fn\tSend output to file fn\n"
				                "\n Output mode modifiers:\n"
				                "  " SWITCH "2\tForce C64 BASIC 2.0 interpretation\n"
				                "  " SWITCH "3\tForce C64 TFC3 interpretation\n"
				                "  " SWITCH "5\tForce C64 Graphics52 interpretation\n"
				                "  " SWITCH "7\tForce C128 BASIC 7.0 interpretation\n"
				                "  " SWITCH "1\tForce C128 BASIC 7.1 interpretation\n"
				                "  " SWITCH "x\tForce X16 BASIC interpretation\n",
				        argv[0]);
				return 0;
				break;

			default:
				fprintf(stderr, "getopt error: %d\n", option);
				break;
		}
	}

	if (None == mode) {		/* missing mode option */
		fprintf(stderr, "No operation mode specified\n"
		                "-- use '%s " SWITCH "h' for help\n",
		        argv[0]);
		return 1;
	}

	if (argc - optind < 1) {	/* missing filenames */
		fprintf(stderr, "Filename missing\n");
		return 1;
	}

	/* If in input mode, and destination file is other than '-' (stdout),
	 * open the output file, else set it to stdout
	 */
	if (In == mode && 0 != strcmp(outfile, "-")) {
		output = fopen(outfile, "at");
		if (NULL == output) {
			output = fopen(outfile, "wt");
			if (NULL == output) {
				fprintf(stderr, "%s: Unable to open output file: %s\n",
				        argv[0], outfile);
				exit(1);
			}
		}
	}
	else {
		output = stdout;
	}

	/* Filename to read first is in argv[optind] */
	for (i = optind; i < argc; i ++) {
		fprintf(stderr, "Processing: %s\n", argv[i]);

		switch (mode) {
			case In:
				if (t64mode)	t642txt(argv[i], output, allfiles, strict);
				else			bas2txt(argv[i], output, allfiles, strict);
				break;

			case Out:
				txt2bas(argv[i], force, t64mode);
				break;

		case None:
		  break;
		}
	}

	/* Close output file, if any */
	if (output != stdout)	fclose(output);
}
