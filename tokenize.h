#pragma once

/* BASIC mode selected */
typedef enum basic_e {
	Any, Basic2, Graphics52, TFC3, Basic7, Basic71, Basic35, Basic4, VicSuper, X16
} basic_t;

int tokenize(const char *input_p, char *output_p, int *length_p, basic_t mode);
int detokenize(const char *input_p, char *output_p, basic_t mode, int strict);
