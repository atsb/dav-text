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
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "features.h"
#include "main.h"
#include "screenIO.h"
#include "fileIO.h"
#include "keyboard.h"
#include "buffers.h"
#include "move.h"

void search()
{
   char *t;
  int offset;
  struct line *l = *currentBuffer->currentLine;
  char down=1; //Whether or not the found word is below the current position
  char count=2;

  if (currentBuffer->head->next->next == currentBuffer->tail && currentBuffer->head->next->length == 1) {
    //No characters in the buffer
    return;
  }
  
  if (helpBarUpdate==0 || strcmp(currentBuffer->searchString,"")==0) {
    displayBottomRow();
    mvaddstr(maxY-1,0,"Search for what word?");
    mvgetstr(maxY-1,22,currentBuffer->searchString);
  }
  if (l == currentBuffer->tail->prev && currentBuffer->cursor.offset >= l->length - 2) {
    l = currentBuffer->head->next;
    offset = 0;
    down = 0;
  }
  else {
    offset = currentBuffer->cursor.offset + 1;
  }
  while (1) {
    t = ( char *)strstr(l->data + offset, currentBuffer->searchString);
    if (t!=NULL) break;
    offset = 0;
    l = l->next;
    if (l==((*currentBuffer->currentLine)->next)) count--;
    if (l==currentBuffer->tail) { 
      l = currentBuffer->head->next; 
      down=0; 
    }
    if (count==0) break;
  }
  
  if (t==NULL) {
    displayBottomRow();
    mvaddstr(maxY-1,0,"String not found.");
    helpBarUpdate=2;
    strcpy(currentBuffer->searchString,"");
    return;
  }
  
  offset = t - l->data;
  
  if (down) {
    while(currentBuffer->cursor.l != l)
      moveDown(&currentBuffer->cursor);
  }
  else {
    while(currentBuffer->cursor.l != l)
      moveUp(&currentBuffer->cursor);
  }
  
  while(currentBuffer->cursor.offset < offset) {
    if (moveRight(&currentBuffer->cursor) == 1) break;
  }

  while(currentBuffer->cursor.offset > offset) {
    if (moveLeft(&currentBuffer->cursor) == 1) break;
  }

  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;
  
  helpBarUpdate = 2;
}

void replace()
{
  static char replaceString[80];
  static char findString[80];
   char *t;
   int offset;
  struct line *l = *currentBuffer->currentLine;
  char down=1; //Whether or not the found word is below the current position
  char count=2;

  if(helpBarUpdate==0 || !strcmp(findString,"") || !strcmp(replaceString,""))
  {
    displayBottomRow();
    mvaddstr(maxY-1, 0, "Replace what?");
    mvgetstr(maxY-1, 14, findString);
    displayBottomRow();
    mvaddstr(maxY-1, 0, "Replace with what?");
    mvgetstr(maxY-1, 19, replaceString);
  }
  offset = currentBuffer->cursor.offset + 1;
  while(1)
  {
    t = ( char *)strstr(l->data + offset, findString);
    if(t!=NULL) break;
    offset = 0;
    l = l->next;
    if(l==((*currentBuffer->currentLine)->next)) count--;
    if(l==currentBuffer->tail) { l = currentBuffer->head->next; down=0; }
    if(count==0) break;
  }

  if(t==NULL)
  {
    displayBottomRow();
    mvaddstr(maxY-1,0,"String not found.");
    helpBarUpdate=2;
    strcpy(findString,"");
    return;
  }

  offset = t - l->data;

  if (down) {
    while(currentBuffer->cursor.l != l)
      moveDown(&currentBuffer->cursor);
  }
  else {
    while(currentBuffer->cursor.l != l)
      moveUp(&currentBuffer->cursor);
  }

  while(currentBuffer->cursor.offset < offset) {
    if (moveRight(&currentBuffer->cursor) == 1) break;
  }

  while(currentBuffer->cursor.offset > offset) {
    if (moveLeft(&currentBuffer->cursor) == 1) break;
  }

  //Delete the findstring
  for(offset = strlen(findString);offset;offset--)
    keyHit(330, 0);

  //Add the replacestring
  for(offset=0;offset<strlen(replaceString);offset++)
    keyHit(replaceString[offset], 0);

  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;

  helpBarUpdate = 2;
}

void tryCompile()
{
  FILE *fp;
  displayBottomRow();
  mvaddstr(maxY-1,0,"Running Makefile...");
  ungetch(' ');
  getch();
  if(strcmp(currentBuffer->fname, "")) save();
  int feature_system_return = system("make 2>dav.err >/dev/null");
  if(feature_system_return)
  {
  	return;
  }
  fp = fopen("dav.err","r");
  fgetc(fp);
  if(feof(fp))
  {
    displayBottomRow();
    mvaddstr(maxY-1,0,"Compile successful.");
    helpBarUpdate = 2;
  }
  else
  {
    displayBottomRow();
    while(currentBuffer->head->next->next != currentBuffer->tail || currentBuffer->head->next->length!=1)
      goToNextBuffer();
    load("dav.err");
    currentBuffer->lineUpdate = currentBuffer->cursor;
    currentBuffer->lineUpdate.lineNum = 0;
    currentBuffer->keepGoing=1;
    helpBarUpdate = 1;
  }
  fclose(fp);
  int feature_system_remove_return = system("rm dav.err");
  if(feature_system_remove_return)
  {
  	return;
  }
}

void gotoLine(int line)
{
  while(currentBuffer->cursor.lineNum < line - 1) {
    if(scrollDown()) {
      break;
    }
  }
  
  while (currentBuffer->cursor.lineNum < line - 1) {
    if (positionDown(&currentBuffer->cursor)) {
      break;
    }
  }
  
  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->keepGoing = 1;
  displayScreen();
}

void toggleAutoIndent()
{
  currentBuffer->lineUpdate.offset = -1;
  autoIndent = !autoIndent;
  displayBottomRow();
  mvaddstr(maxY-1,0,"Auto indenting set to");
  mvaddch(maxY-1,22,autoIndent+48);
  helpBarUpdate = 2;
}

void toggleBottomRow()
{
  bottomRowToggle = !bottomRowToggle;
  helpBar();
  currentBuffer->lineUpdate.offset = -1;
}
