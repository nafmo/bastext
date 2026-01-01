/* main.c
 * - main routines for bastext
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "inmode.h"
#include "outmode.h"
#include "tokenize.h"
#include "utility.h"

#define TRUE 1
#define FALSE 0

#if defined __EMX__ || defined _MSC_VER
#define strcasecmp stricmp
#endif

typedef enum runmode_e { None, In, Out } runmode_t;

void helpscreen(const char *progname);

/* main
 * - main routine
 * evaluates arguments and call the appropriate routines
 */
int main(int argc, char *argv[])
{
	int			option, i;
	int			allfiles = FALSE;
	int			t64mode = FALSE;
	int			p00mode = FALSE;
	int			strict = FALSE;
	runmode_t	mode = None;
	basic_t		force = Any;
	char		*outfile = "-";
	FILE		*output;

#ifdef __EMX__
	/* OS/2 uses '/' as switch character */
	optswchar = SWITCH;
#endif

	/* Operation modes:
	 *  i (in)   - convert from binary to text
	 *  o (out)  - convert from text to binary
	 *  h (help) - print help page
	 *
	 * File format:
	 *  f (format) - select format for input/output file:
	 *    -f t64 - T64 mode   (legacy: -t)
	 *    -d p00 - P00 mode (out mode only; autodetecet in in mode)
	 *
	 * Source format:
	 *  b (basic) - force BASIC mode
	 *    -b 2.0   - BASIC 2.0  (legacy: -2)
	 *    -b 3.5   - BASIC 3.5
	 *    -b 4.0   - BASIC 4.0
	 *    -b 7.0   - BASIC 7.0  (legacy: -7)
	 *    -b 7.1   - BASIC 7.1  (legacy: -1)
	 *    -b 52    - Graphics53 (legacy: -5)
	 *    -b TFC3  - TFC 3      (legacy: -3)
	 *    -b X16   - X16 BASIC
	 *    -b Super - VIC-20 SuperExpander
	 *
	 * Modifiers:
	 *  a (all)    - convert all programs, not only those with recognized start
	 *               address (0401/0801/1001/1201/132D/1C01/4001)
	 *  s (strict) - strict tok64 encoding
	 *  d (dest)   - destination filename
	 *  ? (help)
	 */
	while (-1 != (option = getopt(argc, argv, "iohf:b:asdt23571:?"))) {
		switch (option) {
			case 'i':
				mode = In;
				break;

			case 'o':
				mode = Out;
				break;

			case 'f':
				if (0 == strcasecmp(optarg, "t64")) {
			case 't':
					t64mode = TRUE;
					p00mode = FALSE;
				}
				else if (0 == strcasecmp(optarg, "p00")) {
					t64mode = FALSE;
					p00mode = TRUE;
				}
				break;

			case 'b':
				if (0 == strcmp(optarg, "2.0")) {
			case '2':
					force = Basic2;
					break;
				}
				else if (0 == strcmp(optarg, "3.5")) {
					force = Basic35;
				}
				else if (0 == strcmp(optarg, "4.0")) {
					force = Basic4;
				}
				else if (0 == strcmp(optarg, "7.0")) {
			case '7':
					force = Basic7;
				}
				else if (0 == strcmp(optarg, "7.1")) {
			case '1':
					force = Basic71;
				}
				else if (0 == strcmp(optarg, "52")) {
			case '5':
					force = Graphics52;
				}
				else if (0 == strcasecmp(optarg, "TFC3")) {
			case '3':
					force = TFC3;
				}
				else if (0 == strcasecmp(optarg, "Super")) {
					force = VicSuper;
				}
				else if (0 == strcasecmp(optarg, "X16")) {
					force = X16;
				}
				else {
					fprintf(stderr, "Unrecognized BASIC dialect: %s\n", optarg);
					return 1;
				}
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
				helpscreen(argv[0]);
				return 0;

			default:
				fprintf(stderr, "getopt error: %d\n", option);
				break;
		}
	}

	if (None == mode) {		/* missing mode option */
		helpscreen(argv[0]);
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
				if (t64mode)	t642txt(argv[i], output, allfiles, strict, force);
				else			bas2txt(argv[i], output, allfiles, strict, force);
				break;

			case Out:
				txt2bas(argv[i], force, t64mode ? T64 : (p00mode ? P00 : Prg));
				break;

		case None:
		  break;
		}
	}

	/* Close output file, if any */
	if (output != stdout)	fclose(output);
}

/* Output the help screen */
void helpscreen(const char *progname)
{
	fprintf(stderr,
	        "Usage:\n"
	        "  %s " SWITCH "i|" SWITCH "o [" SWITCH "f TYPE] [" SWITCH "b MODE] [" SWITCH "a] [" SWITCH "s] [" SWITCH "d filename] filename(s)\n"
	        "  %s " SWITCH "h\n"
	        "\n Mode (one of these required):\n"
	        "  " SWITCH "i\tInput mode (binary to text)\n"
	        "  " SWITCH "o\tOutput mode (text to binary)\n"
	        "  " SWITCH "h\tPrint help page\n"
	        "\n File format selection:\n"
	        "  " SWITCH "ft64\tT64 mode (in: reads from specified T64 archive(s)\n"
	        "    \t          out: creates/appends to bastext.t64)\n"
	        "  " SWITCH "fp00\tP00 mode (out: wraps output in .p00 container)\n"
	        "\n BASIC dialect selection:\n"
	        "  " SWITCH "b2.0\tForce C64 BASIC 2.0 interpretation\n"
	        "  " SWITCH "b3.5\tForce C16/+4 BASIC 3.5 interpretation\n"
	        "  " SWITCH "b7.0\tForce C128 BASIC 7.0 interpretation\n"
	        "  " SWITCH "b7.1\tForce C128 BASIC 7.1 interpretation\n"
	        "  " SWITCH "bTFC3\n\tForce C64 TFC3 interpretation\n"
	        "  " SWITCH "b52\tForce C64 Graphics52 interpretation\n"
	        "  " SWITCH "bX16\tEnable Commander X16 BASIC support\n"
	        "  " SWITCH "bSuper\n\tForce VIC-20 SuperExpander BASIC interpretation\n"
	        "\n Behavioural options:\n"
	        "  " SWITCH "a\tConvert all, not just recognized start addresses\n"
	        "    \t (0401/0801/1001/1201/132D/1C01/4001)\n"
	        "  " SWITCH "s\tStrict tok64 compatibility\n"
	        "  " SWITCH "d fn\tSend output to file fn\n",
	        progname,
	        progname);
}
