/* tokens.c
 * - contains a list of C64 BASIC and C128 BASIC tokens
 *   as well as a PETSCII table
 */

#include "tokens.h"

/* C64/VIC20 BASIC 2.0 (base for all versions)
 * offset: 128
 */

const char *c64tokens[] = {
	/* instructions */
	"END",				/* 128 */	/* 0x80 */
	"FOR",
	"NEXT",				/* 130 */
	"DATA",
	"INPUT#",
	"INPUT",
	"DIM",
	"READ",
	"LET",
	"GOTO",
	"RUN",
	"IF",
	"RESTORE",			/* 140 */
	"GOSUB",
	"RETURN",
	"REM",
	"STOP",							/* 0x90 */
	"ON",
	"WAIT",
	"LOAD",
	"SAVE",
	"VERIFY",
	"DEF",				/* 150 */
	"POKE",
	"PRINT#",
	"PRINT",
	"CONT",
	"LIST",
	"CLR",
	"CMD",
	"SYS",
	"OPEN",
	"CLOSE",			/* 160 */	/* 0xA0 */
	"GET",
	"NEW",
	"TAB(",
	"TO",
	"FN",
	"SPC(",
	"THEN",
	"NOT",
	"STEP",

	/* mathematical functions */
	"+",				/* 170 */	/* 0xAA */
	"-",
	"*",
	"/",
	"^", /* (arrow up) */
	"AND",
	"OR",							/* 0xB0 */
	">",
	"=",
	"<",

	/* unary functions */
	"SGN",				/* 180 */	/* 0xB4 */
	"INT",
	"ABS",
	"USR",
	"FRE",
	"POS",
	"SQR",
	"RND",
	"LOG",
	"EXP",
	"COS",				/* 190 */
	"SIN",
	"TAN",							/* 0xC0 */
	"ATN",
	"PEEK",
	"LEN",
	"STR$",
	"VAL",
	"ASC",
	"CHR$",

	/* functions with more than one parameter */
	"LEFT$",			/* 200 */	/* 0xC8 */
	"RIGHT$",
	"MID$",
	
	/* special */
	"GO" /*("GO TO")*/	/* 203 */	/* 0xCB */
};

/* C64 Graphics52 BASIC extension (Software Unlimited)
 * offset: 204
 */

const char *graphics52tokens[] = {
	"SCREEN",			/* 204 */	/* 0xCC */
	"SPRCL",
	"PLOT",
	"DRAW",
	"CLEAR",						/* 0xD0 */
	"TOGL",
	"ERASE",			/* 210 */
	"CHAR",
	"SMOVE",
	"COLOR",
	"SPRITE",
	"SPROG",
	"CPROG",
	"PEN",
	"FLIP",
	"TRANSFER",
	"BLOCK",			/* 220 */
	"BOTTOM",
	"SDP",
	"SCRSV",
	"LOSCR",						/* 0xE0 */
	"LOSPR",
	"LOCHR",
	"SPRSV",
	"CHRSV",
	"SMOOTH",
	"VOLUME",			/* 230 */
	"ADSR",
	"SHIFT",
	"PITCH",
	"WAVE",
	"PULSE",
	"DETECT",
	"PUT",
	"MOVE",
	"PLACE",			/* 240 */	/* 0xF0 */
	"COPY",
	"MEMSV",
	"LOMEM",
	"SWAP",
	"BRD&BKG",
	"SWITCH",
	"UNLESS",
	"MULTI",
	"SHRINK",
	"PADL(",			/* 250 */
	"JOY(",
	"BIT(",
	"LOC(",
	"POINT("
};

/* C64 TFC3 BASIC extension (Riska BV)
 * offset: 204
 */

const char *tfc3tokens[] = {
	"OFF",				/* 204 */	/* 0xCC */
	"AUTO",
	"DEL",
	"RENUM",
	"HELP",							/* 0xD0 */
	"FIND",
	"OLD",				/* 210 */
	"DLOAD",
	"DVERIFY",
	"DSAVE",
	"APPEND",
	"DAPPEND",
	"DOS",
	"KILL",
	"MON",
	"PDIR",
	"PLIST",			/* 220 */
	"BAR",
	"DESKTOP",
	"DUMP",
	"ARRAY",						/* 0xE0 */
	"MEM",
	"TRACE",
	"REPLACE",
	"ORDER",
	"PACK",
	"UNPACK",			/* 230 */
	"MREAD",
	"MWRITE"
};
 
/* C128 BASIC 7.0 and C16/+4 BASIC 3.5
 * offset: 204
 */

const char *c128tokens[] = {
	"RGR",				/* 204 */	/* 0xCC */
	"RCLR",
	"RLUM", /* (prefix in 7.0) */
	"JOY",
	"RDOT",							/* 0xD0 */
	"DEC",
	"HEX$",				/* 210 */
	"ERR$",
	"INSTR",
	"ELSE",
	"RESUME",
	"TRAP",
	"TRON",
	"TROFF",
	"SOUND",
	"VOL",
	"AUTO",				/* 220 */
	"PUDEF",
	"GRAPHIC",
	"PAINT",
	"CHAR",							/* 0xE0 */
	"BOX",
	"CIRCLE",
	"GSHAPE",
	"SSHAPE",
	"DRAW",
	"LOCATE",			/* 230 */
	"COLOR",
	"SCNCLR",
	"SCALE",
	"HELP",
	"DO",
	"LOOP",
	"EXIT",
	"DIRECTORY",
	"DSAVE",
	"DLOAD",			/* 240 */	/* 0xF0 */
	"HEADER",
	"SCRATCH",
	"COLLECT",
	"COPY",
	"RENAME",
	"BACKUP",
	"DELETE",
	"RENUMBER",
	"KEY",
	"MONITOR",			/* 250 */
	"USING",
	"UNTIL",
	"WHILE"				/* 253 */	/* 0xFD */
};

/* C128 BASIC 7.0
 * CE prefix - sprite commands
 */

const char *c128CEtokens[] = {
	"",					/* 0 */		/* 0x0 */
	"",
	"POT",
	"BUMP",
	"PEN",
	"RSPOS",
	"RSPRITE",
	"RSPCOLOR",
	"XOR",
	"RWINDOW"			/* 9 */		/* 0x9 */
};

/* C128 BASIC 7.0
 * FE prefix
 * includes Rick Simon's BASIC 7.1
 */

const char *c128FEtokens[] = {
	"",					/* 0 */		/* 0x0 */
	"",
	"BANK",
	"FILTER",
	"PLAY",
	"TEMPO",
	"MOVSPR",
	"SPRITE",
	"SPRCOLOR",
	"RREG",
	"ENVELOPE",			/* 10 */
	"SLEEP",
	"CATALOG",
	"DOPEN",
	"APPEND",
	"DCLOSE",
	"BSAVE",						/* 0x10 */
	"BLOAD",
	"RECORD",
	"CONCAT",
	"DVERIFY",			/* 20 */
	"DCLEAR",
	"SPRSAV",
	"COLLISION",
	"BEGIN",
	"BEND",
	"WINDOW",
	"BOOT",
	"WIDTH",
	"SPRDEF",
	"QUIT",				/* 30 */
	"STASH",
	"", /* (space) */				/* 0x20 */
	"FETCH",
	"", /* (quote) */
	"SWAP",
	"OFF",
	"FAST",
	"SLOW",				/* 38 */	/* 0x26 */
	/* Rick Simon's BASIC 7.1 extension */
	"CWIND",			/* 39 */	/* 0x27 */
	"SSCRN",			/* 40 */
	"LSCRN",
	"HIDE",
	"SHOW",
	"SFONT",
	"LFONT",
	"VIEW",
	"FCOPY",
	"ESAVE",						/* 0x30 */
	"SEND",
	"CHECK",			/* 50 */
	"ESC",
	"OLD",
	"FIND",
	"DUMP",
	"MERGE"				/* 55 */	/* 0x37 */
};

/* PET BASIC 4.0
 * includes C64 BASIC 4.0 extension
 * offset: 204
 */
const char *basic4tokens[] = {
	"CONCAT",			/* 204 */	/* 0xCC */
	"DOPEN",
	"DCLOSE",
	"RECORD",
	"HEADER",						/* 0xD0 */
	"COLLECT",
	"BACKUP",			/* 210 */
	"COPY",
	"APPEND",
	"DSAVE",
	"DLOAD",
	"CATALOG",
	"RENAME",
	"SCRATCH",
	"DIRECTORY",		/* 218 */	/* 0xDA */
	/* C64 BASIC 4.0 extension */
	"COLOR",			/* 219 */	/* 0xDB */
	"COLD",				/* 220 */
	"KEY",
	"DVERIFY",
	"DELETE",
	"AUTO",							/* 0xE0 */
	"MERGE",
	"OLD",
	"MONITOR"			/* 227 */	/* 0xE3 */
};

/* VIC Super Expander
 * offset: 204
 */
const char *supertokens[] = {
	"KEY",				/* 204 */	/* 0xCC */
	"GRAPHIC",
	"SCNCLR",
	"CIRCLE",
	"DRAW",							/* 0xD0 */
	"REGION",
	"COLOR",			/* 210 */
	"POINT",
	"SOUND",
	"CHAR",
	"PAINT",
	"RPOT",
	"RPEN",
	"RSND",
	"RCOLR",
	"RGR",
	"RJOY",				/* 220 */
	"RDOT"				/* 221 */	/* 0xDD */
};

/* petscii conversion tables
 * singlebyte => characters
 * multibyte => escape sequences (written as {sequence} in the text format)
 */

const char *petscii[] = {
	"null",				/* 0 */		/* 0x0 */
	"ct a",
	"ct b",
	"ct c",
	"ct d",
	"white",
	"ct f",
	"ct g",
	"ct h", /* (disable charset switch (C64)) */
	"ct i", /* (enable charset switch (C64)) */
	"ct j",				/* 10 */
	"ct k",
	"ct l",
	"return",
	"ct n",
	"ct o",
	"ct p",							/* 0x10 */
	"down",
	"reverse on",
	"home",
	"delete",			/* 20 */
	"ct u",
	"ct v",
	"ct w",
	"ct x",
	"ct y",
	"ct z",
	"027", /* (c128) */
	"red",
	"right",
	"green",			/* 30 */
	"blue",
	" ", /* (space) */				/* 0x20 */
	"!",
	"\"",
	"#",
	"$",
	"%",
	"&",
	"'",
	"(",				/* 40 */
	")",
	"*",
	"+",
	",",
	"-",
	".",
	"/",
	"0",							/* 0x30 */
	"1",
	"2",				/* 50 */
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	":",
	";",
	"<",				/* 60 */
	"=",
	">",
	"?",
	"@",							/* 0x40 */
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",				/* 70 */
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"p",				/* 80 */	/* 0x50 */
	"q",
	"r",
	"s",
	"t",
	"u",
	"v",
	"w",
	"x",
	"y",
	"z",				/* 90 */
	"[",
	"pound", /* pound */
	"]",
	"^",
	"arrow left", /* <- */
	"096",							/* 0x60 */
	"097",
	"098",
	"099",
	"100",				/* 100 */
	"101",
	"102",
	"103",
	"104",
	"105",
	"106",
	"107",
	"108",
	"109",
	"110",				/* 110 */
	"111",
	"112",							/* 0x70 */
	"113",
	"114",
	"115",
	"116",
	"117",
	"118",
	"119",
	"120",				/* 120 */
	"121",
	"122",
	"123",
	"124",
	"125",
	"126",
	"127",
	"128",							/* 0x80 */
	"orange",
	"130",				/* 130 */
	"131",
	"132",
	"f1",
	"f3",
	"f5",
	"f7",
	"f2",
	"f4",
	"f6",
	"f8",				/* 140 */
	"141",
	"142",
	"143",
	"black",						/* 0x90 */
	"up",
	"reverse off",
	"clear",
	"148", /* insert */
	"brown",
	"pink",				/* 150 */
	"dark gray",
	"gray",
	"light green",
	"light blue",
	"light gray",
	"156", /* run */
	"left",
	"yellow",
	"cyan",
	"sh space",			/* 160 */	/* 0xA0 */
	"cm k",
	"cm i",
	"cm t",
	"cm @",
	"cm g",
	"cm +",
	"cm m",
	"cm pound",
	"sh pound",
	"cm n",				/* 170 */
	"cm q",
	"cm d",
	"cm z",
	"cm s",
	"cm p",
	"cm a",							/* 0xB0 */
	"cm e",
	"cm r",
	"cm w",
	"cm h",				/* 180 */
	"cm j",
	"cm l",
	"cm y",
	"cm u",
	"cm o",
	"sh @",
	"cm f",
	"cm c",
	"cm x",
	"cm v",				/* 190 */
	"cm b",
	"sh asterisk",					/* 0xC0 */
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",				/* 200 */
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",								/* 0xD0 */
	"Q",
	"R",				/* 210 */
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"sh +",
	"cm -",				/* 220 */
	"sh -",
	"222",
	"cm asterisk",
	"224",								/* 0xE0 */
	"225",
	"226",
	"227",
	"228",
	"229",
	"230",				/* 230 */
	"231",
	"232",
	"233",
	"234",
	"235",
	"236",
	"237",
	"238",
	"239",
	"240",				/* 240 */		/* 0xF0 */
	"241",
	"242",
	"243",
	"244",
	"245",
	"246",
	"247",
	"248",
	"249",
	"250",				/* 250 */
	"251",
	"252",
	"253",
	"254",
	"pi",				/* 255 */		/* 0xFF */
};

/* tok64compatible
 * - checks whether a token that is to be used is tok64 compatible or not
 *   (for strict mode)
 * in:	petscii - petscii character value
 * out:	TRUE / FALSE
 */
int nontok64compatible(int petscii)
{
	return (   (petscii >=   1 && petscii <=   4)
	        || (petscii >=   6 && petscii <=  16)
	        || (petscii >=  20 && petscii <=  26)
	        || (petscii >= 160 && petscii <= 192)
	        || 92 == petscii
	        || 95 == petscii
	        || (petscii >= 219 && petscii <= 221)
	        || 223 == petscii);
}
