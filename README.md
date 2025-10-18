BasText - convert Commodore BASIC to text
==========================================
Copyright 1997-2025 Peter Krefting.
A Softwolves Software Release in 2025

http://www.softwolves.pp.se/sw/

LICENSE
--------

This software is distributed under the GNU General Public License version
2, as can be found in the accompanying file COPYING.

USEAGE
-------

BasText is a program that is used to convert between binary (tokenized)
BASIC files from the Commodore C64 and C128 to a clean ASCII text format
that is human and machine readable, as well as transportable via electronic
mail. This program is designed to be compatible with tok64, while extending
the support for Commodore BASIC extensions and dialects.

The following Commodore BASIC versions and extensions are supported by
this version of BasText:

* Commodore BASIC 2.0 (VIC20/C64)
* Commodore BASIC 3.5 (C16/+4)
* Commodore BASIC 4.0 (PET)
* Commodore BASIC 7.0 (C128)
* Software Unlimited's Graphics52 for Commodore 64
* Riska BV's The Final Cartridge III for Commodore 64
* Rick Simon's BASIC 7.1 for Commodore 128
* VIC Super Extender for VIC-20
* Turtle BASIC 1.0 for VIC-20
* SpeechBASIC 2.7 for Commodore 64
* AtBasic for Commodore 64
* Simon's BASIC for Commodore 64
* Commander X16

BasText tries to autodetect the Commodore BASIC dialect used in the file it
is processing. At the moment, this is done by checking the starting address
of the file. These starting addresses are recognized, and interpreted
(addresses written in hexadecimal):

    $0401
VIC-20 BASIC 2.0 (3K RAM expansion)

    $0801
Commodore 64 BASIC 2.0

    $1001
VIC-20 BASIC 2.0 (unexpanded VIC)

    $1201
VIC-20 BASIC 2.0 (8K RAM expansion)

    $132D
Commodore 128 BASIC 7.1 extension by Rick Simon. This file is a
combined file, with both the BASIC 7.1 extension binary, and the
BASIC source in one file (saved with BASIC 7.1's ESAVE command). The
preamble will be ignored, and the file will be interpreted as BASIC
7.1.

    $1C01
Commodore 128 BASIC 7.0

    $4001
Commodore 128 BASIC 7.0 saved with graphics mode enabled.

In version 1.0, some different third-party extensions were included in the
autodetection. This is not true anymore, and thus you'll need to specify all
extensions via command line parameters.

The text files created by this program are supposed to be compatible with
those of tok64, meaning that the files it creates should be possible to
interpret with this one, and the files this program creates should be
possible to interpret with it. However, an extra header, in front of tok64's
start tok64 header is added, start BasText, which contains the starting
address of the original binary file, as well as the name of BASIC extension
used. This is to make the autodetection possible for the text-to-binary
direction. For Commodore 128 BASIC programs, the header is changed to start
tok128, since tok64 only supports C64 programs, this will make it skip those
programs. For BASIC 4.0 programs, the header is start tokpet, for the same
reason.

When BasText interprets text representations of files back into binary mode,
it will use the start BasText header to determine the BASIC extension to
use. If the BASIC version is not mentioned there (version 1.0 of BasText
only included the start address), BasText tries to autodetect the file
format in the same way as in binary-to-text. If no such header exists (e.g
if the file was created with tok64), BasText will assume that the program's
start address is 0801 (standard for Commodore 64), and that it should
interpret it as Commodore 128 BASIC 7.0, as this is a super-set of BASIC
2.0. This should make BasText able to interpret most text files without any
big problems. If autodetection in this mode does not work, use one of the
"force" parameters described below.

BasText is command line driven, with the following syntax:

    bastext -i [-t] [-a] [-s] [-d filename] filename(s)
    bastext -o [-t] [-2|-3|-5|-7|-1] filename(s)
    bastext -h

One of the three mode selectors must be given:

    -i
Set input mode (converting from binary Commodore tokenized BASIC to
text).

    -o
Set output mode (converting from text to binary Commodore tokenized
BASIC).

    -h
Shows a brief help screen, with an overview of the available options.
These general modifiers (works in both input and output modes) are
available:

    -t
Enable T64 (Commodore 64 emulator tape archive) mode. When in input
mode, this means that instead of the specified file names being binary
Commodore BASIC files, they are T64 archives. When in output mode, this
means that instead of writing the binary Commodore BASIC files to files
in the current directory, they will be written to a T64 archive named
bastext.t64 in the current directory. If the archive already exists, it
will be appended to. The default directory size for the bastext.t64
file is 30 entries. If you try to add more files to it, the program
will abort with an error message. The default directory size is
controlled in the t64.h file.

These modifiers are available only when in input mode:

    -a
Convert all input files, not only those that have a starting address.

    -s
Maintain strict compatibility with tok64. This means that BasText's
"extended" escape codes for charactes 92 (British pound), 95 (left
arrow), 160-192 (shift space to shift asterisk), 219-221 (shift plus,
commodore minus and shift minus), and 223 (commodore asterisk), will be
printed as three-digit numeric escape codes, not as textual escapes.
The "strict" mode will not, however, undo the problems with tok64's
"uppercase in quoted strings"-bug (see under BUGS).

    -d filename
Selects the filename to write the output to. If the filename is not
given, or is given as "-", the listings will be output on the standard
output device (normally the console).

These modifiers are available only when in output mode:

    -2
Force Commodore BASIC 2.0 interpretation of all programs.

    -3
Force Commodore 64 The Final Cartridge III BASIC extension
interpretation of all programs. (See also BUGS).

    -5
Force Commodore 64 Graphics52 BASIC extension interpretation of all
programs.

    -7
Force Commodore 128 BASIC 7.0 interpretation of all programs.

    -1
Force Commodore 128 BASIC 7.1 extension interpretation of all programs.

Please note that the MS-DOS and OS/2 versions (EMX compiled) uses / (slash)
as parameter character.

EXAMPLES
---------

    bastext -i sample.prg

Converts sample.prg to text, and displays it on the standard output.

    bastext -i sample.p00

Converts sample.p00 to text, and displays it on the standard output. The P00
file format is detected automatically.

    bastext -i -s -d programs.txt *.prg

Converts all Commodore BASIC binary files with a prg extension to text,
writing it to programs.txt in the current directory, while maintaining tok64
compatibility.

    bastext -it *.t64 | more

Converts all files in all T64 archives (with filename suffix .t64) in the
current directory into listings, displaying them one page at a time.

    bastext -o7 programs.txt

Converts all programs in the programs.txt text file into Commodore BASIC 7.0
programs.

HISTORY
--------

* v1.0 - 1998-01-18 -
  Initial public release.
* v1.1 - 2023-11-21 -
  Fix incorrect keyboard mapping.
  Fix VICE compatibility.
* v1.2 - unreleased -
  Add support for reading P00 archives.
  Add Commander X16 tokens.

KNOWN BUGS
-----------

tok64 seems to parse uppercase characters in quoted strings incorrectly. It
converts them into characters in the range of 97-122, whereas a Commodore
computer (and bastext) usually uses the "shadow" range of 193-208. This will
not look any different when used on a Commodore computer, but it will make
the binaries differ, and could make a difference if the program needs the
correct PETSCII values. The problem will appear when you convert the program
in one direction with bastext and in the other with tok64, not when using
the same program in both directions.

BasText does not support the "bare" format that tok64 normally outputs
(without the start tok64 header).

CREDITS
-----------

* Thanks to Dirk Jagdmann for patches to update sources for compilers
  20 years newer than what it was originally written for.
* Thanks to Stig Christensen for suggestions for improvements and bug
  reports.
* Thanks to morphinejh for Commander X16 BASIC support.

CONTACT
--------

The author, Peter Krefting, can be contacted
via Internet e-mail at peter@softwolves.pp.se. Information about this
program is available at
http://www.softwolves.pp.se/sw/
