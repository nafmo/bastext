/* t64.c
 * - functions that operates on the T64 files
 */

#include <string.h>
#include <stdio.h>

#include "t64.h"

/* checkvalidheader
 * - checks for T64 file header validity
 * in:	header_p: pointer to header structure
 *		totalentries_p: pointer to where to fill in the max directory size
 *		usedentries_p: pointer to where to fill in the number of used entries
 * out:	the totalentries and usedentries are filled in
 *      zero for valid header
 *      nonzero for invalid header
 */
int checkvalidheader(t64header_t *header_p, unsigned int *totalentries_p,
                     unsigned int *usedentries_p, const char *filename)
{
	/* Locate the strings "C64" and "tape" in the description header */
	if (!strstr(header_p->description, "C64") ||
	    !strstr(header_p->description, "tape")) {
		fprintf(stderr, "File is not a T64 archive: %s\n", filename);
		return 1;
	}

	/* Copy the header data to local data (since this program is supposed to
	 * be runnable independent of if the current architecture is little- or
	 * big-endian, we have to do this).
	 */

	*totalentries_p = header_p->maxfiles[0] | (header_p->maxfiles[1] << 8);
	*usedentries_p  = header_p->numfiles[0] | (header_p->numfiles[1] << 8);

	/* Check for data validity */
	if (0 == *totalentries_p || *usedentries_p > *totalentries_p) {
		fprintf(stderr, "Error in T64 archive header: %s\n", filename);
		return 1;
	}

	return 0;	/* No error */
}
