#pragma once
#include "tokenize.h"

void bas2txt(const char *infile, FILE *output, int allfiles, int strict, basic_t force);
void t642txt(const char *infile, FILE *output, int allfiles, int strict, basic_t force);
