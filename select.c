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
	 *  0401 => BASIC 2.0 (VIC20, +3K RAM) or Graphics52 (C64)
	 *          Graphics52 is the super-set, select it
	 *          NB! PET BASIC 4.0 also uses 0401, but must
	 *          use the -b 4.0 option.
	 *  0801 => BASIC 2.0 (C64) or TFC3 BASIC (C64)
	 *          TFC3 is the super-set, select it
	 *          NB! Commander X16 BASIC also uses 0801, but
	 *          must use the -b X16 option.
	 *  1001 => BASIC 2.0 (VIC20 unexpanded) or BASIC 3.5 (C16/Plus4)
	 *          BASIC 3.5 is the superset
	 *  1201 => BASIC 2.0 (VIC20 +8K RAM)
	 *  132D => BASIC 7.1 (C128) with bound extension file
	 *  1C01 => BASIC 7.0 (C128) or BASIC 7.1 (C128)
	 *          BASIC 7.1 is the super-set, select it
	 *  4001 => BASIC 7.0 (C128)
	 *  other=> select BASIC 7.1 (includes BASIC 2.0 and 7.0)
	 */
	switch (adr) {
		case 0x0401:
			return Graphics52;

		case 0x0801:
			return TFC3;

		case 0x1001:
			return Basic35;

		case 0x1201:
			return Basic2;

		case 0x132D:
		case 0x1C01:
			return Basic71;

		case 0x4001:
			return Basic7;

		default:
			fprintf(stderr, "* Unrecognized start address of BASIC: %04x\n",
			        adr);
			return Basic71;
	}
}
