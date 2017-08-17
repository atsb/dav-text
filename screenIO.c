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
#include <ncurses.h>
#include <string.h>

#include "screenIO.h"
#include "main.h"
#include "fileIO.h"
#include "features.h"
#include "buffers.h"
#include "undo.h"
#include "move.h"

void displayLine(int line, struct position *pos) {
  int ch;
  unsigned char *data = pos->l->data + pos->offset;
  int length = pos->l->length;
  unsigned char *end = pos->l->data + length;   
  int screenX = 0;
  
  if(pos->l != currentBuffer->tail) {
    while(1) {
      ch = *data++;
      if(ch==9) { 
        /* Tab */
        while((screenX % tabWidth) != tabWidth-1) {
          mvaddch(line, screenX++, ' ');
        }
      }
      mvaddch(line, screenX, ch);
      if (screenX++ >= maxX || data==end) { 
        screenX--; 
        break; 
      }
    }
  }

  length = lastDisplayed[line];
  lastDisplayed[line] = screenX;
  while (screenX<length) {
    mvaddch(line, screenX++, ' ');
  }
  
}

void displayScreen() {
  int lineNum = currentBuffer->lineUpdate.lineNum;

  if(currentBuffer->lineUpdate.offset == -1) {
    return;
  }
  
  if (displayWholeScreen) {
    int i;
    for (i=0; i<maxY; i++) {
      lastDisplayed[i] = maxX;
    }
    displayWholeScreen = 0;
  }
  
  while(currentBuffer->lineUpdate.cursX) {
    moveLeft(&currentBuffer->lineUpdate);
  }

  do {
    displayLine(lineNum++, &currentBuffer->lineUpdate);
  } while(!positionDown(&currentBuffer->lineUpdate) && currentBuffer->lineUpdate.cursY<maxY-1);
  
  if(currentBuffer->keepGoing) {
    while(lineNum<maxY-1) {
      displayLine(lineNum, &currentBuffer->lineUpdate);
      if(positionDown(&currentBuffer->lineUpdate)==1) {
        currentBuffer->lineUpdate.l = currentBuffer->tail;
      }
      lineNum++;
    }
  }
  
  currentBuffer->keepGoing = 0;
}

void showRow() {
  char num[5];
  char total[5];
  int t;
  num[0] = ((currentBuffer->cursor.lineNum+1)/10000) % 10 + 48;
  num[1] = ((currentBuffer->cursor.lineNum+1)/1000) % 10 + 48;
  num[2] = ((currentBuffer->cursor.lineNum+1)/100) % 10 + 48;
  num[3] = ((currentBuffer->cursor.lineNum+1)/10) % 10 + 48;
  num[4] = (currentBuffer->cursor.lineNum+1) % 10 + 48;

  total[0] = ((currentBuffer->numLines+1)/10000) % 10 + 48;
  total[1] = ((currentBuffer->numLines+1)/1000) % 10 + 48;
  total[2] = ((currentBuffer->numLines+1)/100) % 10 + 48;
  total[3] = ((currentBuffer->numLines+1)/10) % 10 + 48;
  total[4] = (currentBuffer->numLines+1) % 10 + 48;
  for (t=0; t<4; t++) { 
    if(num[t]==48) { 
      num[t]=' ';
    } else {
      break; 
    }
  }
  for (t=0; t<4; t++) { 
    if(total[t]==48) {
      total[t]=' '; 
    } else {
      break;
    }
  }
  mvaddstr(maxY-1,maxX-11,num);
  mvaddch(maxY-1,maxX-6,'/');
  mvaddstr(maxY-1,maxX-5,total);
}

void helpBar() {
  char c[200];
  char *ptr = c;
  int t;
  displayBottomRow();
  
  if(bottomRowToggle) {
    strcpy(ptr,"F|");
    ptr += 2;
    for(t=0;t<12;t++)
    {
      if(t<=8)
        *ptr++ = t + '1';
      else
      {
        *ptr++ = '1';
        *ptr++ = t + '1' - 10;
      }
      *ptr++ = ':';
      if(Fn_ptr[t] == search) { 
        strcpy(ptr, "Search");
        ptr+=6; 
      } else if(Fn_ptr[t] == save) {
        strcpy(ptr, "Save");
        ptr+=4; 
      } else if(Fn_ptr[t] == saveAs) {
        strcpy(ptr, "SaveAs");
        ptr+=6; 
      } else if(Fn_ptr[t] == askLoad) {
        strcpy(ptr, "Load");
        ptr+=4; 
      } else if(Fn_ptr[t] == tryQuit) {
        strcpy(ptr, "Quit");
        ptr+=4; 
      } else if(Fn_ptr[t] == Undo) {
        strcpy(ptr, "Undo");
        ptr+=4; 
      } else if(Fn_ptr[t] == tryCompile) {
        strcpy(ptr, "Compile");
        ptr+=7; 
      } else if(Fn_ptr[t] == toggleAutoIndent) { 
        strcpy(ptr, "AutoIndent");
        ptr+=10; 
      } else if(Fn_ptr[t] == toggleBottomRow) { 
        strcpy(ptr, "BottomRow");
        ptr+=9; 
      } else if(Fn_ptr[t] == replace) { 
        strcpy(ptr, "Replace");
        ptr+=7; 
      } else if(Fn_ptr[t] == goToNextBuffer) { 
        strcpy(ptr, "Next");
        ptr+=4; 
      } else if(Fn_ptr[t] == goToPrevBuffer) { 
        strcpy(ptr, "Prev");
        ptr+=4; 
      } else if(Fn_ptr[t] == nothing) { 
        ptr-=3; 
      }
      *ptr++ = '|';
    }
    *ptr = '\0';
    mvaddnstr(maxY-1,0,c,maxX-1);
  } else {
    char temp[3];
    temp[0] = (currentBufferNum / 10) + '0';
    temp[1] = (currentBufferNum % 10) + '0';
    temp[2] = '\0';
    strcpy(c,"Editing ");
    strcat(c,currentBuffer->fname);
    strcat(c," [");
    if(currentBufferNum >= 10) {
      strcat(c,temp);
    } else {
      strcat(c,temp+1);
    }
    strcat(c,"]");
    mvaddstr(maxY-1,0,c);
  }
  showRow();
}

void displayBottomRow() {
  int x;
  for(x=maxX-12; x; x--) {
    mvaddch(maxY-1, x, ' ');
  }
}
