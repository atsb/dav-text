/*
Copyright 2001-2003 David Gucwa

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
#ifndef structs_h
#define structs_h

struct line {
  unsigned char *data;
  unsigned int length;
  struct line *next;
  struct line *prev;
  unsigned char hasTabs;
};

struct position {
  struct line *l;
  unsigned int offset;
  unsigned int lineNum;
  unsigned int cursX,cursY;
  unsigned int wantCursX;
};

struct undoMove {
  /* Reverses some modification to a buffer */
  unsigned int line, offset;
  char c; /* What was added or removed */
};

struct buffer {
  char searchString[80];
  int numLines;
  struct line *head,*tail;
  struct position cursor;
  struct position topLine;
  struct position lineUpdate;
  char keepGoing;
  struct line **currentLine;
  char fname[80];
  int lineNumBak;
  char updated;
  struct undoMove *undoMoves;
  int undoBufferPointer;
  int undoBufferLength;
};

#endif
