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
#include <stdlib.h>
#include <string.h>

#include "keyboard.h"
#include "main.h"
#include "screenIO.h"
#include "undo.h"
#include "move.h"

void keyHit(int keypress, char undoNow)
{
  int t=0,t2=0;
  int x;
  static char ctrl=0;
   char *ptr;
   char **data;

  if(keypress==27) { 
    ctrl=3; 
    return; 
  }
  if(keypress==79 && ctrl==3) { 
    ctrl--; 
    return; 
  }
  if(keypress==99 && ctrl==2) {
    currentBuffer->lineUpdate.offset = -1;
    ctrl=0;
    helpBarUpdate = 1;
    data = &((*currentBuffer->currentLine)->data);
    t=currentBuffer->cursor.cursY;
    currentBuffer->lineUpdate = currentBuffer->cursor;
    currentBuffer->lineUpdate.lineNum = currentBuffer->cursor.cursY;
    while((*data)[currentBuffer->cursor.offset]!=' ' && (*data)[currentBuffer->cursor.offset]!='\n'
    && (*data)[currentBuffer->cursor.offset]!=9)
      if(moveRight(&currentBuffer->cursor)) break;
    while((*data)[currentBuffer->cursor.offset]==' ' || (*data)[currentBuffer->cursor.offset]=='\n'
    || (*data)[currentBuffer->cursor.offset]==9)
      if(moveRight(&currentBuffer->cursor)) break;
    return;
  }
  if(keypress==100 && ctrl==2) {
    currentBuffer->lineUpdate.offset = -1;
    ctrl=0;
    helpBarUpdate = 1;
    data = &((*currentBuffer->currentLine)->data);
    t=currentBuffer->cursor.cursY;
    currentBuffer->lineUpdate = currentBuffer->cursor;
    currentBuffer->lineUpdate.lineNum = currentBuffer->cursor.cursY;
    moveLeft(&currentBuffer->cursor);
    while((*data)[currentBuffer->cursor.offset]==' ' || (*data)[currentBuffer->cursor.offset]=='\n'
    || (*data)[currentBuffer->cursor.offset]==9)
      if(moveLeft(&currentBuffer->cursor)) break;
    while((*data)[currentBuffer->cursor.offset]!=' ' && (*data)[currentBuffer->cursor.offset]!='\n'
    && (*data)[currentBuffer->cursor.offset]!=9)
      if(moveLeft(&currentBuffer->cursor)) break;
    moveRight(&currentBuffer->cursor);
    return;
  }
  if(keypress==91 && ctrl==3) { 
    ctrl--; 
    return; 
  }
  if(keypress==55 && ctrl==2) { 
    ctrl=1; 
    return; 
  }
  else if((keypress==94 && ctrl==1) || keypress==391)
  {
    currentBuffer->cursor.cursY = currentBuffer->cursor.cursX = currentBuffer->cursor.offset = 0;
    currentBuffer->cursor.l = currentBuffer->head->next;
    currentBuffer->topLine = currentBuffer->cursor;
    currentBuffer->lineUpdate = currentBuffer->cursor;
    currentBuffer->lineUpdate.lineNum = 0;
    currentBuffer->keepGoing=1;
    ctrl=0;
    return;
  }
  else if(keypress==335 || keypress==386)
  {
    *currentBuffer->currentLine = currentBuffer->tail->prev;
    currentBuffer->cursor.offset = (*currentBuffer->currentLine)->length - 1;
    determineCursX(&currentBuffer->cursor);
    currentBuffer->cursor.cursY=0;
    currentBuffer->topLine=currentBuffer->cursor;
    currentBuffer->topLine.cursX = currentBuffer->topLine.offset = 0;
    for(t=0;t<maxY-4;t++)
      scrollUp();
    currentBuffer->lineUpdate=currentBuffer->topLine;
    currentBuffer->lineUpdate.lineNum=0;
    currentBuffer->keepGoing=1;
    return;
  }
  ctrl=0;
  switch(keypress)
  {
    case 410:
      displayBottomRow();
      break;
    case -1: /* Window Resizing */
      {
        struct line *tempLine = currentBuffer->cursor.l;
        t = currentBuffer->cursor.offset;
        
        /* Reset to top left */
        keyHit(262, 0); /* Home key */
        while (currentBuffer->cursor.cursY > 0) {
          keyHit(259, 0); /* Up key */
        }
        
        getmaxyx(stdscr,maxY,maxX);
        free(lastDisplayed);
        lastDisplayed = (int *)malloc(maxY*sizeof(int));
        memset(lastDisplayed,0,maxY*sizeof(int));
        helpBar();
        while (currentBuffer->cursor.l != tempLine) {
          keyHit(258, 0); /* Down key */
        }
        while (currentBuffer->cursor.offset < t) {
          keyHit(261, 0); /* Right key */
        }
        while (currentBuffer->cursor.offset > t) {
          keyHit(260, 0); /* Left */
        }
        currentBuffer->lineUpdate = currentBuffer->topLine;
        currentBuffer->lineUpdate.lineNum = 0;
        currentBuffer->keepGoing = 1;
        
        /* Tell the display to refresh the whole screen */
        displayWholeScreen = 1;
        
        break;
      }

    case 265: //Function keys
    case 266:
    case 267:
    case 268:
    case 269:
    case 270:
    case 271:
    case 272:
    case 273:
    case 274:
    case 275:
    case 276:
      currentBuffer->lineUpdate.offset = -1;
      (*Fn_ptr[keypress-265])();
      break;

    case 258: //Down
      currentBuffer->lineUpdate.offset = -1;
      moveDown(&currentBuffer->cursor);
      if(smartCursor) {
        while(currentBuffer->cursor.wantCursX > currentBuffer->cursor.cursX
              && currentBuffer->cursor.cursX < ((*currentBuffer->currentLine)->length % maxX) - 1) {
          moveRight(&currentBuffer->cursor);
        }
      }
      break;

    case 259: //Up
      currentBuffer->lineUpdate.offset = -1;
      moveUp(&currentBuffer->cursor);
      if(smartCursor) {
        while(currentBuffer->cursor.wantCursX > currentBuffer->cursor.cursX
              && currentBuffer->cursor.cursX < ((*currentBuffer->currentLine)->length % maxX) - 1) {
          moveRight(&currentBuffer->cursor);
        }
      }
      break;

    case 260: /* Left */
      currentBuffer->lineUpdate.offset = -1;
      moveLeft(&currentBuffer->cursor);
      currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      break;

    case 261: /* Right */
      currentBuffer->lineUpdate.offset = -1;
      moveRight(&currentBuffer->cursor);
      currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      break;

    case 338: /* PgDn */
      currentBuffer->lineUpdate.offset = -1;
      x = currentBuffer->cursor.cursY + 2;
      for(t=maxY-1; t; t--) {
        moveDown(&currentBuffer->cursor);
      }
      for(t=0; t<maxY-x; t++) {
        scrollDown();
      }
      if(smartCursor) {
        while(currentBuffer->cursor.wantCursX > currentBuffer->cursor.cursX
           && currentBuffer->cursor.cursX < (*currentBuffer->currentLine)->length - 1) {
          moveRight(&currentBuffer->cursor);
        }
      }
      
      break;

    case 339: //PgUp
      currentBuffer->lineUpdate.offset = -1;
      x = currentBuffer->cursor.cursY;
      for(t=0;t<maxY-1;t++)
        moveUp(&currentBuffer->cursor);
      for(t=0;t<x;t++)
        scrollUp();
      if(smartCursor) {
        while(currentBuffer->cursor.wantCursX > currentBuffer->cursor.cursX
           && currentBuffer->cursor.cursX < (*currentBuffer->currentLine)->length - 1) {
          moveRight(&currentBuffer->cursor);
        }
      }
      break;

    case 262: //Home
      currentBuffer->lineUpdate.offset = -1;
      while(currentBuffer->cursor.cursX) moveLeft(&currentBuffer->cursor);
      currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      break;

    case 360: //End
      t2=currentBuffer->topLine.lineNum;
      while(1) {
        t=moveRight(&currentBuffer->cursor);
        if(t==1) break;
        else if(t>1) {
          moveLeft(&currentBuffer->cursor);
          break;
        }
      }
      currentBuffer->lineUpdate.offset = -1;
      if(t2!=currentBuffer->topLine.lineNum) scrollUp();
      currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      break;

    case 263: //Backspace
    case 8: //Shift- or Ctrl-Backspace
      if(*currentBuffer->currentLine==currentBuffer->head->next && currentBuffer->cursor.offset==0) {
        currentBuffer->lineUpdate.offset = -1;
        break;
      }
      moveLeft(&currentBuffer->cursor);
      //Continue on to delete

    case 330: //Delete
      if((*currentBuffer->currentLine)->next==currentBuffer->tail && currentBuffer->cursor.offset==(*currentBuffer->currentLine)->length-1) break;
      if(undoNow) Do(0);
      if((*currentBuffer->currentLine)->data[currentBuffer->cursor.offset]==9) (*currentBuffer->currentLine)->hasTabs--;
      x = ((*currentBuffer->currentLine)->data[currentBuffer->cursor.offset]=='\n');
      (*currentBuffer->currentLine)->length--;

      ptr = (*currentBuffer->currentLine)->data;
      memmove(ptr + currentBuffer->cursor.offset, ptr + currentBuffer->cursor.offset + 1, currentBuffer->cursor.l->length - currentBuffer->cursor.offset);
      (*currentBuffer->currentLine)->data = ( char *)realloc((*currentBuffer->currentLine)->data, (*currentBuffer->currentLine)->length);

      currentBuffer->lineUpdate = currentBuffer->cursor;
      currentBuffer->lineUpdate.lineNum = currentBuffer->cursor.cursY;
      if(!((*currentBuffer->currentLine)->length%maxX)) currentBuffer->keepGoing=1;
      if(x) { connectLines(*currentBuffer->currentLine); currentBuffer->keepGoing=1; }
      currentBuffer->updated = 1;
      currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      break;

    case 21: //Ctrl-U
      keyHit(262,undoNow); //Home
      for(x=(*currentBuffer->currentLine)->length;x;x--) {
        keyHit(330,undoNow); //Delete
      }
      break;

    case 11: //Ctrl-K
      if(currentBuffer->cursor.offset==(*currentBuffer->currentLine)->length - 1) {
        keyHit(330,undoNow);
        break;
      }
      x = currentBuffer->cursor.cursX;
      keyHit(360, undoNow); //End
      if(currentBuffer->cursor.l->data[currentBuffer->cursor.offset] != '\n') {
        keyHit(330, undoNow); //Delete
      }
      while(currentBuffer->cursor.cursX > x) {
        keyHit(263, undoNow); //Backspace
      }
      break;

    case 13: //Enter
      if(undoNow) Do(1);
      currentBuffer->updated = 1;
      currentBuffer->cursor.cursY++;
      currentBuffer->cursor.cursX = 0;
      currentBuffer->cursor.lineNum++;

      if(autoIndent) {
        for(t2=0;(*currentBuffer->currentLine)->data[t2]==9 && t2<currentBuffer->cursor.offset;t2++);
        for(t=t2;(*currentBuffer->currentLine)->data[t]==' ' && t<currentBuffer->cursor.offset;t++);
        t-=t2;
      }

      addLineAfter(*currentBuffer->currentLine, "");
      x = (*currentBuffer->currentLine)->length;

      ptr = ( char *)malloc(x - currentBuffer->cursor.offset);
      memmove(ptr, (*currentBuffer->currentLine)->data + currentBuffer->cursor.offset, x - currentBuffer->cursor.offset);
      free((*currentBuffer->currentLine)->next->data);
      (*currentBuffer->currentLine)->next->data = ptr;
      (*currentBuffer->currentLine)->next->length = x - currentBuffer->cursor.offset;

      (*currentBuffer->currentLine)->data = ( char *)realloc((*currentBuffer->currentLine)->data, currentBuffer->cursor.offset + 1);
      (*currentBuffer->currentLine)->length = currentBuffer->cursor.offset + 1;

      (*currentBuffer->currentLine)->data[(*currentBuffer->currentLine)->length - 1] = '\n';
      *currentBuffer->currentLine = (*currentBuffer->currentLine)->next;
      currentBuffer->cursor.offset = 0;
      countTabs(*currentBuffer->currentLine);
      countTabs((*currentBuffer->currentLine)->next);
      if(autoIndent) {
        for(;t2;t2--) keyHit(9, undoNow);
        for(;t;t--) keyHit(' ',undoNow);
      }
      currentBuffer->lineUpdate=currentBuffer->cursor;
      do {
        moveLeft(&currentBuffer->lineUpdate);
      } while(currentBuffer->lineUpdate.offset);
      do {
        moveLeft(&currentBuffer->lineUpdate);
      } while(currentBuffer->lineUpdate.offset);
      currentBuffer->lineUpdate.lineNum = currentBuffer->lineUpdate.cursY;
      currentBuffer->keepGoing=1;
      if(currentBuffer->cursor.cursY==maxY-1) scrollDown();
      currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      break;

    default: //Anything that adds to the text
      if(undoNow) Do(1);
      currentBuffer->lineUpdate=currentBuffer->cursor;
      currentBuffer->lineUpdate.lineNum = currentBuffer->cursor.cursY;
      if(!((*currentBuffer->currentLine)->length % maxX)) currentBuffer->keepGoing = 1;

      if(keypress==9) //Tab
      {
        currentBuffer->cursor.cursX += tabWidth - 1 - (currentBuffer->cursor.cursX % tabWidth);
        (*currentBuffer->currentLine)->hasTabs++;
      }

      x = (*currentBuffer->currentLine)->length;

      (*currentBuffer->currentLine)->data = realloc((*currentBuffer->currentLine)->data, x + 1);
      memmove((*currentBuffer->currentLine)->data + currentBuffer->cursor.offset + 1, (*currentBuffer->currentLine)->data + currentBuffer->cursor.offset, x - currentBuffer->cursor.offset);
      (*currentBuffer->currentLine)->data[currentBuffer->cursor.offset] = ( char)keypress;

      (*currentBuffer->currentLine)->length++;
      currentBuffer->cursor.offset++;
      currentBuffer->cursor.cursX++;
      if(currentBuffer->cursor.cursX>=maxX)
      {
        currentBuffer->cursor.cursX = 0;
        if(++currentBuffer->cursor.cursY>=maxY-1) scrollDown();
      }
      currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      currentBuffer->updated = 1;
  }
}

void listChoice(int n, char **choices, char *answer, char *message)
{
  int start = 0;
  int end;
  int selected = 0;
  int t;
  int line;
  int keypress;
  char newName[256];
  int x,y;
  char focus = 0;

  newName[0] = '\0';

  while(1)
  {  
    for(y=0;y<maxY;y++)
      for(x=0;x<maxX;x++)
        mvaddch(y,x,' ');

    line = 0;
    end = (start + maxY - 1 >= n) ? n : start + maxY - 1;
    for(t=start;t<end;t++)
    {
      mvaddstr(line, 2, choices[t]);
      line++;
    }
    if(!focus) mvaddstr(selected - start, 0, "->");
    mvaddstr(maxY-1, 0, message);
    mvaddstr(maxY-1, strlen(message)+1, newName);

    keypress = getch();
    if(keypress == 258) { //Down
      focus = 0;
      if(selected < n-1) selected++;
      if(selected - start == maxY-1)
        start++;
    }
    else if(keypress == 259) { //Up
      focus = 0;
      if(selected) selected--;
      if(selected < start) start--;
    }
    else if(keypress == 13) { //Enter
      strcpy(answer, focus ? newName : choices[selected]);
      if(focus == 0 && selected == 0) //Special case, cancel
        strcpy(answer, "DAV_CANCEL");
      break;
    }
    else if(keypress == 263) { //Backspace
      focus = 1;
      if(strlen(newName) != 0)
        newName[strlen(newName) - 1] = '\0';
    }
    else if(keypress == 338) { //PgDn
      focus = 0;
      for(t=0;t<maxY;t++) {
        if(selected < n-1) selected++;
        if(selected - start == maxY-1)
          start++;
      }
    }
    else if(keypress == 339) { //PgUp
      focus = 0;
      for(t=0;t<maxY;t++) {
        if(selected) selected--;
        if(selected < start) start--;
      }
    }
    else if(keypress == 410) { //Window Resizing

    }
    else if(keypress == -1) { //Window Resizing
      getmaxyx(stdscr,maxY,maxX);
      free(lastDisplayed);
      lastDisplayed = (int *)malloc(maxY*sizeof(int));
      memset(lastDisplayed,0,maxY*sizeof(int));
      while(selected - start >= maxY-1)
        start++;
    }
    else {
      focus = 1;
      newName[strlen(newName) + 1] = '\0';
      newName[strlen(newName)] = keypress;
    }
  }

  for(y=0;y<maxY;y++)
    for(x=0;x<maxX;x++)
      mvaddch(y,x,' ');
}
