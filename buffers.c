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
#include "buffers.h"
#include "screenIO.h"
#include <string.h>

extern int numberOfBuffers;
extern int helpBarUpdate;
extern char smartCursor;

extern struct buffer *buffers;
extern struct buffer *currentBuffer;
extern int currentBufferNum;

void keyHit(int keypress, char undoNow);
void quit(char *text);

void goToNextBuffer()
{
  currentBufferNum++;
  if(currentBufferNum == numberOfBuffers) currentBufferNum = 0;
  currentBuffer = &buffers[currentBufferNum];
  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;
  helpBarUpdate=1;
}

void goToPrevBuffer()
{
  currentBufferNum--;
  if(currentBufferNum == -1) currentBufferNum = numberOfBuffers - 1;
  currentBuffer = &buffers[currentBufferNum];
  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;
  helpBarUpdate=1;
}

void closeBuffer()
{
  struct buffer *b = currentBuffer;
  char tempSmartCursor = smartCursor;
  smartCursor = 0;

  //Go to the very beginning of the buffer
  while(currentBuffer->cursor.l!=currentBuffer->head->next || currentBuffer->cursor.offset!=0)
    keyHit(259, 0); //Key_Up

  //Wipe out lines
  while(currentBuffer->numLines)
    keyHit(21, 0); //Ctrl-U
  keyHit(21, 0); //Last remaining line

  currentBuffer->updated = 0;
  strcpy(currentBuffer->fname, "");
  
  while(strcmp(currentBuffer->fname, "") == 0 && !currentBuffer->updated) {
    goToPrevBuffer();
    if(currentBuffer == b) quit("");
  }
  smartCursor = tempSmartCursor;
  //helpBarUpdate = 1;
  //currentBuffer->lineUpdate.offset = -1;
  displayScreen();
}
