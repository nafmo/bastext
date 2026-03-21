#pragma once
#include "tokenize.h"

/* Input/output mode selected */
typedef enum outmode_e {
    Prg, T64, P00, D64
} outmode_t;

void txt2bas(const char *infile, basic_t force, outmode_t outmode);
