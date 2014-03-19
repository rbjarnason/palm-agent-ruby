/* Copyright (c) 2000-2002, Jacco Bikker
 * Copyright (c) 2000-2001, Ben Burton
 * Copyright (c) 2002, InOrbit Entertainment, Inc.
 *
 * This file is part of PalmAlice.
 * PalmAlice is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the 
 * Free Software Foundation; either version 2, or (at your option) any later 
 * version.
 *
 * PalmAlice is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.

 * You should have received a copy of the GNU General Public License along 
 * with PalmAlice; see the file COPYING. If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#define palmDBType 'DATA'
#define palmDBCreator 'CILA'
#define palmVersion 0
#define NUM_VARS 35

#define prefsCurrentAlice 0

typedef struct {
	UInt dbCard;
	LocalID dbID;
	UInt aliceIndex;
} AliceLocation;

typedef struct {
	AliceLocation location;
	Boolean exists;
} CurrentAliceData;

typedef struct {
	VoidHand alice;
	VoidHand source;
} AliceContents;

extern CurrentAliceData currentAlice;

// Alice();
// ~Alice();
Boolean FGetS(FileHand stream, Char* buffer, Int32 objSize);
void print( char* s);
void tidyup( char* text );
void analyzer( void );
Boolean match( char* text, char* pattern, Boolean final );
void person( char* text, char* pstring, char* pfile );
void reevaluate( char* line );
int wordcount( char* text );
void randomize( char* text );
Boolean trysecond( void );
void tokenizer( char* text );
Boolean condition( char* text );
void respond( char* text, char* bot );
Boolean checkfiles( char* bot );
Boolean checkdict();
void savevars( char* varfile );
void loadvars( char* varfile );
void initialize( void );
int calcmood( char* text );
void updatemood( char* text );
void greeting( void );
void execute( char* command );
void InitAlice();

// void setchunk( int idx, long& pos ) { chunk[idx] = pos; } 
// char* username( void ) { return var[8]; }
int mainloop( void );
char* prevstr, *prevbuf;

// char* star, *star2, *that, *buffer, *second, *justthat, *beforethat;
char* star, *star2, *that, *second, *justthat, *beforethat;
char** defaultvar;
char** var;
char** moodvar;
char** moodcalc;
char** substitutes;
char* readBuffer;
char* replaceBuffer;
char* part1;
char* part2;
char* part3;
char* reetemp;
char* reebuffer;
char* fname;
char* tempsaves;
char* spaces;
char* rstring;
char* randbuffer;
char* tryline;
char* tag;

char* substbuffer;
char* substline;
char* subststring1;
char* subststring2;

char* respondpos;
MemHandle respondposHandle;

char* personBuffer;
char* cleanerBuffer;
char* removeTemp1;
char* outputBuffer;

char* emess;
char* pluseoln, *texteoln;	// Used in match
char* agenda1, *agenda2;	// Agenda item (to be confirmed)
char** saves;

char* bufferMain;

int tx[32], tx2[32], ty[32], ty2[32], tc[32], curt;
int sage[10];
char** setvars, **getvars;
UInt16 pateval, tokens;
UInt8 recursion;
Boolean newvarfile, exitplease, repetitive, garbage;
float happyness, helpfullness, humor, affection, trust, interest;

UInt32 lastrun, lastquit, thisrun;
UInt32 wakeup, secdelay;

UInt32 chunk[27];
char* user_input, **token, *userstr;
Boolean topic[5];

UInt8 recurs;

int random( int max );
int strval( char* text ); 
void delay( int msecs );
void remove( char* text, char* first, char* last );
void bzero( char* text, int len );
void uppercase( char* text );

void lowercase( char* text );

Boolean replace( char* line, char* string1, char* string2 );
void cleaner( char* text );
int count( char* text, char* subst );
void substitute( char* text, char* file );

void printb(Char *newString);
void printbMore(Char *newString);
void printbSwap();
void printMore(Char *newStr);
void DrawSplashScreen();
void CleanInit();
void saveinitstate();
unsigned long RandomNum(unsigned long n);
void DrawRandomImage(Boolean force);
void DrawImage(int x, int y, UInt16 RscId);