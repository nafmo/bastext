#pragma once
#include <stdint.h>
#include <stddef.h>

/* BASIC mode selected */
typedef enum basic_e {
	Any, Basic2, Graphics52, TFC3, Basic7, Basic71, Basic35, Basic4, VicSuper, X16
} basic_t;

int tokenize(const char *input_p, uint8_t *output_p, int *length_p, basic_t mode);
int detokenize(const uint8_t *input_p, ptrdiff_t len, FILE *output, basic_t mode, int strict);
