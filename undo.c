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
#include "undo.h"
#include "main.h"
#include "keyboard.h"
#include "move.h"

void Do(int keypress)
{
  switch(keypress)
  {
    case 0:
      addToUndo((int)currentBuffer->cursor.l->data[currentBuffer->cursor.offset]);
      break;

    case 1:
      addToUndo(0);
      break;

    default:
      break;
  }
}  

void addToUndo(int keypress)
{
  currentBuffer->undoMoves[currentBuffer->undoBufferPointer].c = keypress;
  currentBuffer->undoMoves[currentBuffer->undoBufferPointer].line = currentBuffer->cursor.lineNum;
  currentBuffer->undoMoves[currentBuffer->undoBufferPointer].offset = currentBuffer->cursor.offset;
  currentBuffer->undoBufferPointer++;
  currentBuffer->undoBufferPointer %= maxUndoLength;
  if(++currentBuffer->undoBufferLength>maxUndoLength) currentBuffer->undoBufferLength--;
}

void Undo()
{
  int key;
  if(!undoEnabled) return;
  if(currentBuffer->undoBufferLength==0) return;
  currentBuffer->undoBufferLength--;
  if(--currentBuffer->undoBufferPointer == -1) currentBuffer->undoBufferPointer=maxUndoLength-1;
  key = (int)currentBuffer->undoMoves[currentBuffer->undoBufferPointer].c;
  while(currentBuffer->cursor.lineNum > currentBuffer->undoMoves[currentBuffer->undoBufferPointer].line)
    moveUp(&currentBuffer->cursor);
  while(currentBuffer->cursor.lineNum < currentBuffer->undoMoves[currentBuffer->undoBufferPointer].line)
    moveDown(&currentBuffer->cursor);
  while(currentBuffer->cursor.offset < currentBuffer->undoMoves[currentBuffer->undoBufferPointer].offset)
    moveRight(&currentBuffer->cursor);
  while(currentBuffer->cursor.offset > currentBuffer->undoMoves[currentBuffer->undoBufferPointer].offset)
    moveLeft(&currentBuffer->cursor);

  if(key=='\n') key=13;
  if(key == 0) key = 330;
  keyHit(key,0);
  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;
}
