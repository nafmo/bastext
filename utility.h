// Copyright Peter Krefting
// Written on 2000-04-16
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

#if defined __EMX__ || defined _MSC_VER
# define SWITCH "/"
#else
# define SWITCH "-"
#endif

#ifdef __EMX__
#include <getopt.h>
#elif _POSIX_C_SOURCE >= 2 || _XOPEN_SOURCE || defined __DARWIN_UNIX03
#include <unistd.h>
#else
/** Index to next non-option parameter. */
extern int optind;

/** Pointer to option argument. */
extern char *optarg;

/**
 * Retrieve command line options.
 * @param _argc Original argc parameter passed to main.
 * @param _argv Pointer to the argv array passed to main.
 * @param opts  String describing valid option characters. Suffix option
 *              character with colon (":") to indicate that the option takes
 *              an argument (stored in optarg).
 * @return Next option on command line, or '?' if an invalid option was found.
 */
int getopt(int _argc, char **_argv, const char *opts);
#endif
