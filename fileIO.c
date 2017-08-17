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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <ncurses.h>

#include "fileIO.h"
#include "structs.h"
#include "screenIO.h"
#include "keyboard.h"
#include "main.h"

extern struct buffer *currentBuffer;
extern int helpBarUpdate;
extern int maxY;
extern char autoIndent;

void save()
{
  if(!strcmp(currentBuffer->fname, "")) {
    doSave(".");
  }
  else {
    doSave(currentBuffer->fname);
  }
  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->lineUpdate.cursY = 0;
  currentBuffer->keepGoing=1;
}

void saveAs()
{
  char *ptr = (char *)malloc(strlen(currentBuffer->fname));
  strcpy(ptr, currentBuffer->fname);
  if(!strcmp(doSave("."), "")) {
    strcpy(currentBuffer->fname, ptr);
  }
  free(ptr);
  currentBuffer->lineUpdate = currentBuffer->topLine;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->lineUpdate.cursY = 0;
  currentBuffer->keepGoing=1;
}

char *doSave(char *filename) {
  int t;
  FILE *fp;
  struct line *l = currentBuffer->head->next;
  struct stat s;

  helpBarUpdate = 1;

  if(!strcmp(filename, ""))
    return "";                                                                                              

  stat(filename, &s);

  if(S_ISDIR(s.st_mode)) {
    while(1) {
      char newFile[256];
      DIR *dir = opendir(filename);
      char **choices = malloc(sizeof(char *));
      int n = 1;
      char answer[256];
      struct dirent *d;

      choices[0] = malloc(7);
      strcpy(choices[0], "Cancel");
      while(1)
      {
        d = readdir(dir);
        if(d==NULL) break;
        n++;
        choices = realloc(choices, n*sizeof(char *));
        choices[n-1] = malloc(256);
        sprintf(choices[n-1], d->d_name);
      }
      closedir(dir);
      
      if (optimize == 0) { //Memory efficiency
        randomizedQuickSort(&choices[1], 0, n-2);
      }
      else { //Cpu efficiency
        radixSort(&choices[1], n-1);
      }
      
      listChoice(n, choices, answer, "File to save as?");
      if(!strcmp(answer, "DAV_CANCEL")) {
        newFile[0] = '\0';
      }
      else if(!strcmp(answer, ".")) {
        sprintf(newFile, "%s", filename);
      }
      else if(!strcmp(answer, "..")) {
        sprintf(newFile, "%s/%s", filename, answer);
      }
      else {
        sprintf(newFile, "%s/%s", filename, answer);
      }
      for(n--;n>=0;n--) {
        free(choices[n]);
      }
      free(choices);
      return doSave(newFile);
    }
  }

  fp = fopen(filename, "w");
  helpBarUpdate = 2;
  if(fp==NULL)
  {
    mvaddstr(maxY-1,0,
    "You do not have the proper permissions to save to that file");
    return "";
  }
  while(l!=currentBuffer->tail->prev)
  {
    for(t=0;t<l->length;t++)
      putc(l->data[t],fp);
    l = l->next;
  }
  for(t=0;t<l->length-1;t++)
    putc(l->data[t],fp);
  fclose(fp);
  displayBottomRow();
  mvaddstr(maxY-1,0,"File successfully saved.");
  strcpy(currentBuffer->fname, filename);
  currentBuffer->updated = 0;
  return currentBuffer->fname;
}

void load(char *filename)
{
  char t;
  int i;
  struct stat s;
  FILE *fp;
  stat(filename, &s);

  if(!strcmp(filename, ""))
    return;

  if(S_ISDIR(s.st_mode)) //File is a directory
  {
    DIR *dir = opendir(filename);
    char **choices = malloc(sizeof(char *));
    int n = 1;
    char answer[256];
    char newFile[256];
    struct dirent *d;

    choices[0] = malloc(7);
    strcpy(choices[0], "Cancel");

    while(1)
    {
      d = readdir(dir);
      if(d==NULL) break;
      n++;
      choices = realloc(choices, n*sizeof(char *));
      choices[n-1] = malloc(256);
      sprintf(choices[n-1], d->d_name);
    }
    closedir(dir);
    
    if (optimize == 0) { //Memory efficiency
      randomizedQuickSort(&choices[1], 0, n-2);
    } 
    else { //Cpu efficiency
      radixSort(&choices[1], n-1);
    }
    
    listChoice(n, choices, answer, "File to load?");
    if(!strcmp(answer, "DAV_CANCEL"))
    {
      currentBuffer->lineUpdate = currentBuffer->topLine;
      currentBuffer->keepGoing=1;
      currentBuffer->lineUpdate.lineNum = 0;
      currentBuffer->lineUpdate.cursY = 0;
      newFile[0] = '\0';
    }
    else if(!strcmp(answer, ".")) {
      sprintf(newFile, "%s", filename);
    }
    else if(!strcmp(answer, "..")) {
      sprintf(newFile, "%s/%s", filename, answer);
    }
    else {
      sprintf(newFile, "%s/%s", filename, answer);
    }
    for(n--;n>=0;n--)
      free(choices[n]);
    free(choices);
    load(newFile);
    return;
  }
  fp = fopen(filename,"r");

  strcpy(currentBuffer->fname, filename);
  if(fp==NULL)
  {
    displayBottomRow();
    mvaddstr(maxY-1,0,"Couldn't open file.");
    helpBarUpdate=2;
    return;
  }

  //Go to the very beginning of the buffer
  while(currentBuffer->cursor.l!=currentBuffer->head->next || currentBuffer->cursor.offset!=0)
    keyHit(259, 0); //Key_Up

  //Wipe out lines
  while(currentBuffer->numLines)
    keyHit(21, 0); //Ctrl-U
  keyHit(21, 0); //Last remaining line

  //Turn off autoindenting and read from the file
  t = autoIndent;
  autoIndent = 0;
  i = (int)getc(fp);
  if(i=='\n') i=13;
  while(i!=EOF)
  {
    keyHit(i,0);
    i = (int)getc(fp);
    if(i=='\n') i=13;
  }
  fclose(fp);
  autoIndent = t;

  //Go back up to the top
  while(currentBuffer->cursor.l!=currentBuffer->head->next || currentBuffer->cursor.offset!=0)
    keyHit(259, 0); //Key_Up

  currentBuffer->lineUpdate = currentBuffer->cursor;
  currentBuffer->keepGoing=1;
  currentBuffer->lineUpdate.lineNum = 0;
  currentBuffer->lineUpdate.cursY = 0;
  currentBuffer->updated = 0;
}

void askLoad()
{
  load(".");
  helpBarUpdate = 1;
}
