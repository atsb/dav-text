/*
Copyright 2001-2003 David Gucwa
Copyright 2017-2018 Adam Bilbrough

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
#include "move.h"
#include "main.h"

void moveDown(struct position *p) {
  positionDown(p);
  if(p->cursY==maxY-1 && p==&currentBuffer->cursor) { 
    scrollDown(); 
  }
}

void moveUp(struct position *p) {
  positionUp(p);
  if(p->cursY==-1 && p==&currentBuffer->cursor) { 
    scrollUp(); 
  }
}

char moveLeft(struct position *p) {
  /*
  Returns 0 upon normal movement
  Returns 1 if the position could not be moved left
  Returns 2 if moving left caused the position to enter the previous line
  Returns 3 if moving left caused the position to wrap on the same line
  */
  
  char r = 0;
  if(p->l == currentBuffer->head->next && p->offset == 0) {
    /* Beginning of file */
    return 1;
  }
  if(!p->offset) {
  /* Beginning of line */
    p->l = p->l->prev;
    p->offset = p->l->length - 1;
    if(p->l->hasTabs) {
      determineCursX(p);
    }
    else {
      p->cursX = p->offset % maxX;
    }
    p->cursY--;
    p->lineNum--;
    r = 2;
  }
  else {
    p->offset--;
    if(!p->cursX) {
      r = 3;
      p->cursY--;
      if(p->l->hasTabs) {
        determineCursX(p);
      }
      else {
        p->cursX = maxX - 1;
      }
    }
    else {
      if(p->l->hasTabs)
        determineCursX(p);
      else
        p->cursX--;
    }

  }
  if(p->cursY==-1 && p==&currentBuffer->cursor) { 
    scrollUp(); 
  }
  return r;
}

char moveRight(struct position *p) {
  /* 
  Returns 0 upon normal movement
  Returns 1 if the position could not be moved right (end of file)
  Returns 2 if moving right caused the position to enter the next line
  Returns 3 if moving right caused the position to wrap on the same line
  */

  char r=0;
  if(p->l->next == currentBuffer->tail && p->offset == p->l->length-1) {
    /* End of file */
    return 1;
  }
  
  if(p->offset < p->l->length - 1) {
    /* Normal movement */
    int temp = p->cursX;
    p->offset++;
    if (p->l->hasTabs) {
      determineCursX(p);
    } else {
      p->cursX++;
      if (p->cursX == maxX) {
        p->cursX=0;
      }
    }
    if (p->cursX < temp) {
      p->cursY++;
      r = 3;
    }
  }
  else {
    /* Next line */
    p->l = p->l->next;
    p->cursX = p->offset = 0;
    p->cursY++;
    p->lineNum++;
    r = 2; //Hit a new line
  }
  if(p->cursY == maxY-1 && p==&currentBuffer->cursor) { 
    scrollDown(); 
  }
  return r;
}

char scrollDown()
{
  /* Returns 1 if the screen did not scroll down */

  if(currentBuffer->topLine.lineNum + (maxY>>1) > currentBuffer->numLines) {
    return 1;
  }

  if(currentBuffer->cursor.cursY==0) {
    positionDown(&currentBuffer->cursor);
  }

  if(!positionDown(&currentBuffer->topLine)) {
    currentBuffer->cursor.cursY--;
  }

  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;
  return 0;
}

char scrollUp()
{
  /* Returns 1 if the screen did not scroll up */
  if(currentBuffer->cursor.cursY==maxY-1) {
    positionUp(&currentBuffer->cursor);
  }
  
  if(!positionUp(&currentBuffer->topLine)) {
    currentBuffer->cursor.cursY++;
  }
  
  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;
  return 0;
}
