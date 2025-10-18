#pragma once

/* C64 BASIC 2.0 */
extern const char *c64tokens[];

/* C64 Graphics52 BASIC extension (Software Unlimited) */
extern const char *graphics52tokens[];

/* C64 TFC3 BASIC extension (Riska BV) */
extern const char *tfc3tokens[];

/* C128 BASIC 7.0
 * includes Rick Simon's BASIC 7.1 extensions
 * includes C16/+4 BASIC 3.5
 */
extern const char *c128tokens[];
extern const char *c128CEtokens[];
extern const char *c128FEtokens[];

/* PET BASIC 4.0
 * includes C64 BASIC 4.0 extension
 */
extern const char *basic4tokens[];

/* VIC Super Extender
 */
extern const char *supertokens[];

/* Commander X16 BASIC (r48)
 */
extern const char *x16tokens[];

/* PETSCII */
extern const char *petscii[];
int nontok64compatible(int petscii);
