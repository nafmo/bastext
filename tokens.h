/* tokens.h
 */

#ifndef __TOKENS_H
#define __TOKENS_H

/* C64 BASIC 2.0 */
extern const char *c64tokens[];

/* C64 Graphics52 BASIC extension (Software Unlimited) */
extern const char *graphics52tokens[];

/* C64 TFC3 BASIC extension (Riska BV) */
extern const char *tfc3tokens[];

/* C128 BASIC 7.0
 * includes Rick Simon's BASIC 7.1 extensions
 */
extern const char *c128tokens[];
extern const char *c128CEtokens[];
extern const char *c128FEtokens[];

/* PETSCII */
extern const char *petscii[];
int nontok64compatible(int petscii);

#endif