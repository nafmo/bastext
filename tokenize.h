/* tokenize.h
 */

#ifndef __TOKENIZE_H
#define __TOKENIZE_H

/* BASIC mode selected */
typedef enum basic_e {
	Any, Basic2, Graphics52, TFC3, Basic7, Basic71
} basic_t;

int tokenize(const char *input_p, char *output_p, int *length_p, basic_t mode);
int detokenize(const char *input_p, char *output_p, basic_t mode, int strict);

#endif