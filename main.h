/*
Copyright 2001-2003 David Gucwa
Copyright 2017-2019 Adam Bilbrough

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef main_h
#define main_h

#include <stdio.h>
#include <curses.h>

#include "structs.h"

char scrollDown();
char scrollUp();
void doArguments(int argc, char *argv[]);
void doAlternativeArguments(int argc, char *argv[]);
void quit(char *text);
void displayHelp();
void displayVersion();
void sigcatch();
void loadSettings();
void writeRC(FILE *fp);
void tryQuit();
void nothing();
void logMsg(char *msg);
void radixSort(char **strings, int number);
void randomizedQuickSort(char **strings, int low, int high);

typedef void (*ptrToFunction)();

void addLineAfter(struct line *whichLine, char *data);
void connectLines(struct line *baseline);
void determineLineNum(struct position *p);
void countTabs(struct line *l);
void determineCursX(struct position *p);
ptrToFunction Fn_ptr[12]; //Bindings for the Fn keys; Fx = Fn_ptr[x-1]

extern int maxY,maxX;
extern int helpBarUpdate;

extern char *version;

//Preferences and default values
extern int maxUndoLength;
extern char undoEnabled;
extern char autoIndent;
extern int numberOfBuffers;
extern char bottomRowToggle;
extern char bufferQuit;
extern char tabWidth;
extern char smartCursor;
extern char optimize;

extern int *lastDisplayed; /* Number of characters last displayed on various lines */
extern char displayWholeScreen; /* Whether to refresh the whole screen */

extern struct buffer *buffers;
extern struct buffer *currentBuffer;
extern int currentBufferNum;

#endif
