/* select.c
 */

#include <stdio.h>

#include "select.h"
#include "tokenize.h"

/* selectbasic
 * - Selects a BASIC dialect with regard to the starting address
 * in:	adr - starting address
 * out:	BASIC dialect
 */
basic_t selectbasic(int adr)
{
	/* With regard to the starting address, select a probable
	 * BASIC version
	 *  0401 => BASIC 2.0 (VIC20) or Graphics52 (C64)
	 *          Graphics52 is the super-set, select it
	 *  0801 => BASIC 2.0 (C64) or TFC3 BASIC (C64)
	 *          TFC3 is the super-set, select it
	 *  132D => BASIC 7.1 (C128) with bound extension file
	 *  1C01 => BASIC 7.0 (C128) or BASIC 7.1 (C128)
	 *          BASIC 7.1 is the super-set, select it
	 *  4001 => BASIC 7.0 (C128)
	 *  other=> select BASIC 7.1 (includes BASIC 2.0 and 7.0)
	 */
	switch (adr) {
		case 0x0401:
			return Graphics52;
			break;

		case 0x0801:
			return TFC3;
			break;

		case 0x132D:
		case 0x1C01:
			return Basic71;
			break;

		case 0x4001:
			return Basic7;
			break;

		default:
			fprintf(stderr, "* Unrecognized start address of BASIC: %04x\n",
			        adr);
			return Basic71;
			break;
	}
}