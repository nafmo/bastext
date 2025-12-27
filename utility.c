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

#if _POSIX_C_SOURCE >= 2 || _XOPEN_SOURCE
#elif defined __EMX__
#else
#include "utility.h"

#include <string.h>
#include <stdio.h>

int optind = 0;
char *optarg = NULL;

// Retrieve command line parameters
int getopt(int _argc, char **_argv, const char *opts)
{
    static int curarg = 1, curind = 0;

redo:
    // Have we reached the last or a non-option argument?
    if (curarg >= _argc || (0 == curind && *_argv[curarg] != *SWITCH))
    {
        optind = curarg;
        return EOF;
    }

    // Check for -- and end if so
    if (0 == curind && *SWITCH == _argv[curarg][1])
    {
        optind = curarg + 1;
        return EOF;
    }

    // Skip dash
    if (0 == curind) curind ++;

    // Move to next argument if at end
    if (0 == _argv[curarg][curind])
    {
        curarg ++;
        optind = curarg;
        curind = 0;
        goto redo;
    }

    // Check this option
    int option = (int) (unsigned char) _argv[curarg][curind];

    const char *p;
    if (NULL != (p = strchr(opts, option)))
    {
        // Does it take an argument?
        if (':' == *(p + 1))
        {
            // Yes.
            // More characters in this parameter?
            curind ++;
            if (_argv[curarg][curind] == '\0')
            {
                // No, next parameter is argument
                if (curarg + 1 >= _argc)
                {
                    fprintf(stderr, "Option requires an argument: %c\n", (char) option);
                    return (int) (unsigned char) '?';
                }
                optarg = _argv[curarg + 1];
                curarg += 2;
                curind = 0;
            }
            else
            {
                // Yes, rest of this parameter is argument
                optarg = &_argv[curarg][curind];
                curarg ++;
                curind = 0;
            }
        }
        else
        {
            // No.
            // Point to next character for next run
            curind ++;
            optarg = NULL;
        }
        return option;
    }
    curind ++;

    // Unknown
    return (int) (unsigned char) '?';
}
#endif
