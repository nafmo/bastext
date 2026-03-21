#pragma once
#include <stdbool.h>
#include "tokenize.h"

void bas2txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force);
void t642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force);
void d642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force);
