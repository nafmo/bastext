#pragma once
#include <stdbool.h>
#include "tokenize.h"

void bas2txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force, bool uppercsae);
void t642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force, bool uppercase);
void d642txt(const char *infile, FILE *output, bool allfiles, bool strict, basic_t force, bool uppercase);
