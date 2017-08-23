/*
Copyright 2001-2003 David Gucwa
Copyright 2017 Adam Bilbrough

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

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/dir.h>

#include "main.h"
#include "buffers.h"
#include "structs.h"
#include "fileIO.h"
#include "screenIO.h"
#include "keyboard.h"
#include "features.h"
#include "undo.h"
#include "move.h"

int maxY,maxX;
int helpBarUpdate=0;

char *version = "Dav - 0.8.6\n\nCopyright 2001-2003 David Gucwa\nCopyright 2017 Adam Bilbrough";
char *license = "This program is free software under the GPL version 2 license.";
                
/* Preferences and default values */
int maxUndoLength = 500;
char undoEnabled=1;
char autoIndent=1;
int numberOfBuffers = 10;
char bottomRowToggle = 1;
char bufferQuit = 0;
char tabWidth = 8;
char smartCursor = 1;
char optimize = 0;

int *lastDisplayed; //Number of characters last displayed on various lines
char displayWholeScreen = 0;

struct buffer *buffers;
struct buffer *currentBuffer;
int currentBufferNum = 0;


int main(int argc, char *argv[])
{
  int x, y, i;
  int keypress;

  signal(2,sigcatch);

  Fn_ptr[0] = search;
  Fn_ptr[1] = save;
  Fn_ptr[2] = saveAs;
  Fn_ptr[3] = askLoad;
  Fn_ptr[4] = tryQuit;
  Fn_ptr[5] = Undo;
  Fn_ptr[7] = toggleAutoIndent;
  Fn_ptr[8] = tryCompile;
  Fn_ptr[9] = toggleBottomRow;
  loadSettings();

  if(numberOfBuffers > 100) numberOfBuffers = 100;  
  buffers = (struct buffer *)malloc(numberOfBuffers * sizeof(struct buffer));
  
  //Set up initial buffers
  for(x=0;x<numberOfBuffers;x++)
  {
    buffers[x].head = (struct line *)malloc(sizeof(struct line));
    buffers[x].tail = (struct line *)malloc(sizeof(struct line));
    buffers[x].head->next = buffers[x].tail;
    buffers[x].head->data = NULL;
    buffers[x].tail->prev = buffers[x].head;
    buffers[x].tail->next = buffers[x].tail;
    buffers[x].tail->data = NULL;
    buffers[x].tail->hasTabs = 0;

    currentBuffer = &buffers[x];
    addLineAfter(buffers[x].head, " ");

    buffers[x].cursor.l = buffers[x].head->next;
    buffers[x].cursor.offset = 0;
    buffers[x].cursor.lineNum = 0;
    buffers[x].topLine.l = buffers[x].head->next;
    buffers[x].topLine.offset = 0;
    buffers[x].topLine.lineNum = 0;
    buffers[x].topLine.cursX = buffers[x].topLine.cursY = 0;
    buffers[x].cursor.cursX = buffers[x].cursor.cursY = 0;
    buffers[x].cursor.wantCursX = 0;
    buffers[x].currentLine = &buffers[x].cursor.l;
    buffers[x].lineUpdate.offset = -1;

    buffers[x].numLines = 0;
    buffers[x].updated = 0;
    if(undoEnabled) buffers[x].undoMoves = (struct undoMove *)malloc(maxUndoLength*sizeof(struct undoMove));
    buffers[x].undoBufferPointer = buffers[x].undoBufferLength = 0;
  }

  currentBuffer = &buffers[0];
 
  initscr();
  cbreak();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
  getmaxyx(stdscr,maxY,maxX);
  
  for(x = 0; x< maxX; x++) {
    for(y = 0; y< maxY; y++) {
      mvaddch(y, x,' ');
    }
  }
  
  move(0,0);
  lastDisplayed = (int *)malloc(maxY*sizeof(int));
  
  /* For Debian issue #872848 ~Adam */
  for (i = 0; i < argc; i++) {
  if (argv[i][0] == '-') {
  	doArguments(argc,argv);
  }
  if (argv[i][0] == '+') {
  	doAlternativeArguments(argc,argv);
  }
  }

  helpBar();
  showRow();
  
  while(1)
  {
    move(currentBuffer->cursor.cursY,currentBuffer->cursor.cursX);
    keypress=getch();
    keyHit(keypress,undoEnabled);
    displayScreen();
    showRow();
    if(helpBarUpdate)
    {
      helpBarUpdate--;
      if(!helpBarUpdate) helpBar();
    }
  }
  return 0;
}

void nothing()
{
  currentBuffer->lineUpdate.offset = -1;
}

void tryQuit()
{
  int t;
  if(!currentBuffer->updated) {
    if(!bufferQuit) quit("");
    else closeBuffer();
    return;
  }
  displayBottomRow();
  mvaddstr(maxY-1,0,"Save before quit? y/n/[C]");
  move(maxY-1,26);
  t = getch();
  mvaddch(maxY-1,26,' ');
  if(t=='y' || t=='Y') { 
    save(); 
    t = 'n';
  }
  if(t=='n' || t=='N') {
    if(!bufferQuit) quit("");
    else closeBuffer();
  }
  helpBarUpdate = 1;
  currentBuffer->lineUpdate.offset = -1;
}

void quit(char *text)
{
  struct line *l;
  int t;

  for(t=0;t<numberOfBuffers;t++) {
    if(t==currentBufferNum) continue;
    if(buffers[t].updated) {
      displayBottomRow();
      mvaddstr(maxY-1,0,"One or more buffers have not been saved. Quit? y/[N]");
      t = getch();
      if(t=='y' || t=='Y') {
        break;
      }
      else {
        helpBarUpdate = 1;
        return;
      }
    }
  } 

  for(t=0;t<maxX;t++) {
    mvaddch(maxY-1,t,' ');
  }
  nodelay(stdscr, TRUE);
  getch();
  endwin();
  
  if(undoEnabled) {
    for(t=0;t<numberOfBuffers;t++) {
      free(buffers[t].undoMoves);
    }
  }
  
  for(t=0;t<numberOfBuffers;t++) {
    l = buffers[t].head;
    while(l != buffers[t].tail) { 
      l=l->next; 
      if(l->prev->data) free(l->prev->data); 
      free(l->prev); 
    }  
    free(buffers[t].tail);
  }
  
  free(buffers);
  free(lastDisplayed);
  printf("%s",text);
  exit(0);
}

void doArguments(int argc, char *argv[])
{
  int x;
  char firstFile = 1;
  for(x=1;x<argc;x++)
  {
    if(argv[x][0] != '-') {
      /* Specifying a file name */
      if(!firstFile) {
        goToNextBuffer();
      }      
      load(argv[x]);
      firstFile = 0;
    } else {
      /* Option */
      if(!strcmp(argv[x],"--help")) {
        displayHelp();
      } else if(!strcmp(argv[x],"--version")) {
        displayVersion();
        /* For Debian issue #872848 ~Adam */
      } else if(strstr(argv[x], "-l") != NULL && isdigit(argv[x][2])) {
        gotoLine(atoi(&argv[x][2]));
      } else {
        /* Mistyped something */
        displayHelp();
      }
  }
  }
  
  /* Go to first buffer */
  while (currentBuffer != &buffers[0]) {
    goToPrevBuffer();
  }
  
  displayScreen();
}

/* For Debian issue #872848 ~Adam */
void doAlternativeArguments(int argc, char *argv[])
{
 int x;
  char firstFile = 1;
  for(x=1;x<argc;x++)
  {
    if(argv[x][0] != '+') {
      /* Specifying a file name */
      if(!firstFile) {
        goToNextBuffer();
      }      
      load(argv[x]);
      firstFile = 0;
    } else { 
      /* Option */
      if(strstr(argv[x], "+") != NULL && isdigit(argv[x][1])) {
        gotoLine(atoi(&argv[x][1]));
      } else {
        /* Mistyped something */
        displayHelp();
      }
    }    
  }
  
  /* Go to first buffer */
  while (currentBuffer != &buffers[0]) {
    goToPrevBuffer();
  }
  
  displayScreen();
}

void displayVersion()
{
  char text[120];
  sprintf(text, "%s\n", version);
  sprintf(text, "%s\n", license);
  quit(text);
}

void displayHelp()
{
  char *c = getenv("HOME");
  int t;
  endwin();
  printf("%s\n",version);
  printf("Usage: dav [arguments] [FILENAME] [FILENAME] ...\n");
  printf("  where FILENAMEs, if specified, are the names of the files you wish to load.\n");
  printf("Arguments list:\n");
  printf("  --help : Display this help screen\n");
  printf("  --version: Display the version of Dav that you are running\n");
  printf("  -l#: Initialize Dav at a specific line number. (eg -l123)\n");
  printf("  +#: Same as -l# but using nano style. (eg +42)\n");
  printf("Basic commands:\n");
  for(t=0;t<12;t++)
  {
    if(!Fn_ptr[t] || Fn_ptr[t]==nothing)
      continue;
    printf("  F%i : ",t+1);
    if(Fn_ptr[t] == search)
      printf("Search\n");
    else if(Fn_ptr[t] == replace)
      printf("Find and replace\n");
    else if(Fn_ptr[t] == save)
      printf("Save current file\n");
    else if(Fn_ptr[t] == saveAs)
      printf("Save current file, prompt for filename\n");
    else if(Fn_ptr[t] == askLoad)
      printf("Load file from within Dav\n");
    else if(Fn_ptr[t] == tryQuit)
      printf("Quit (ask for save if needed)\n");
    else if(Fn_ptr[t] == Undo)
      printf("Undo last keypress\n");
    else if(Fn_ptr[t] == toggleAutoIndent)
      printf("Toggle auto-indenting\n");
    else if(Fn_ptr[t] == tryCompile)
      printf("Compile and print error messages\n");
    else if(Fn_ptr[t] == toggleBottomRow)
      printf("Toggle help bar\n");  
    else if(Fn_ptr[t] == goToNextBuffer)
      printf("Switch to next text buffer\n");
    else if(Fn_ptr[t] == goToPrevBuffer)
      printf("Switch to previous text buffer\n");  
    
  }
  printf("  Ctrl-C : Quit (won't ask for save)\n");
  printf("  Ctrl-K : Erase to end of line\n");
  printf("  Ctrl-U : Erase whole line\n");
  printf("Personal options:\n");
  printf("  Located in %s/.davrc\n",c);
  printf("  Edit %s/.davrc to customize function key bindings\n",c);
  initscr();
  quit("");
}

void loadSettings()
{
  int l;
  char s[80];
  char home[80];
  char *r;
  char *c;
  FILE *fp;
  int gotten=0;
  char FnGotten[12];
  for(l=0;l<12;l++)
    FnGotten[l] = 0;
  strcpy(home,getenv("HOME"));
  strcat(home,"/.davrc");
  fp = fopen(home,"r");
  if(fp==NULL) //File doesn't exist, make one
  {
    fp = fopen(home,"w");
    if(fp==NULL) { return; }
    writeRC(fp);
    fclose(fp);
    fp = fopen(home,"r");
  }
  //It's there, so read from it
  while(!feof(fp)) {
    fgets(s, 200, fp);
    if(s[0]=='#') continue;
    r = strtok(s," =");    
    c = strtok(NULL," =");
    if(c==NULL) continue;
    l = atoi(c);
    if(!strcmp(r,"Undo")) { undoEnabled = l; gotten|=1; }
    if(!strcmp(r,"UndoBuffer")) { maxUndoLength = l; gotten|=2; }
    if(!strcmp(r,"autoIndent")) { autoIndent = l; gotten|=4; }
    if(!strcmp(r,"helpBarInit")) { bottomRowToggle = l; gotten|=8; }
    if(!strcmp(r,"buffers")) { numberOfBuffers = l; gotten|=16; }
    if(!strcmp(r,"bufferQuit")) { bufferQuit = l; gotten|=32; }
    if(!strcmp(r,"tabWidth")) { tabWidth = l; gotten|=64; }
    if(!strcmp(r,"smartCursor")) { smartCursor = l; gotten|=128; }
    if(!strcmp(r,"optimize")) { optimize = l; gotten|=256; }
    if(r[0] == 'F')
    {
      c[strlen(c)-1]  = '\0';
      r++;
      l = atoi(r);
      l--;
      //Function key binding
      if(!strcmp(c, "SEARCH"))
      {
        Fn_ptr[l] = search;
        FnGotten[l] = 1;
      }
      else if(!strcmp(c, "SAVE"))
      {
        Fn_ptr[l] = save;
        FnGotten[l] = 2;
      }
      else if(!strcmp(c, "SAVE_AS"))
      {
        Fn_ptr[l] = saveAs;
        FnGotten[l] = 3;
      }
      else if(!strcmp(c, "LOAD"))
      {
        Fn_ptr[l] = askLoad;
        FnGotten[l] = 4;
      }
      else if(!strcmp(c, "QUIT"))
      {
        Fn_ptr[l] = tryQuit;
        FnGotten[l] = 5;
      }
      else if(!strcmp(c, "UNDO"))
      {
        Fn_ptr[l] = Undo;
        FnGotten[l] = 6;
      }
      else if(!strcmp(c, "COMPILE"))
      {
        Fn_ptr[l] = tryCompile;
        FnGotten[l] = 7;
      }
      else if(!strcmp(c, "TOGGLE_AUTOINDENT"))
      {
        Fn_ptr[l] = toggleAutoIndent;
        FnGotten[l] = 8;
      }
      else if(!strcmp(c, "TOGGLE_BOTTOM_ROW"))
      {
        Fn_ptr[l] = toggleBottomRow;
        FnGotten[l] = 9;
      }
      else if(!strcmp(c, "NOTHING"))
      {
        Fn_ptr[l] = nothing;
        FnGotten[l] = 10;
      }
      else if(!strcmp(c, "REPLACE"))
      {
        Fn_ptr[l] = replace;
        FnGotten[l] = 11;
      }
      else if(!strcmp(c, "PREV"))
      {
        Fn_ptr[l] = goToPrevBuffer;
        FnGotten[l] = 12;
      }
      else if(!strcmp(c, "NEXT"))
      {
        Fn_ptr[l] = goToNextBuffer;
        FnGotten[l] = 13;
      }
      else 
      {
        Fn_ptr[l] = nothing;
        FnGotten[l] = 0;
      }
    }
  }
  fclose(fp);
  for(l=0;l<12;l++)
  {
    if(FnGotten[l]==0)
      gotten = 0;
  }
  if(gotten!=511)
  {
    fp = fopen(home,"w");
    if(fp==NULL) return;
    writeRC(fp);
    fclose(fp);
  }
}

void writeRC(FILE *fp)
{
  int t;
  fprintf(fp,"#Set this to 1 if you want undoing enabled, otherwise set it to 0.\n");
  fprintf(fp,"#Having the undo feature enabled uses a little more processor power.\n");
  fprintf(fp,"Undo = %i\n\n",undoEnabled);
  fprintf(fp,"#This determines how big your undo buffer will be.\n");
  fprintf(fp,"#Actual memory usage in bytes is the following number times nine.\n");
  fprintf(fp,"UndoBuffer = %i\n\n",maxUndoLength);
  fprintf(fp,"#Set this to 1 if you want auto indenting enabled, otherwise set it to 0.\n");
  fprintf(fp,"#If auto indent is enabled, new lines will be indented.\n");
  fprintf(fp,"autoIndent = %i\n\n",autoIndent);
  fprintf(fp,"#This line will determine whether the help bar is enabled by default.\n");
  fprintf(fp,"#You can toggle it any time by hitting F10 inside Dav.\n");
  fprintf(fp,"helpBarInit = %i\n\n",bottomRowToggle);
  fprintf(fp,"#How many text buffers will Dav use?\n");
  fprintf(fp,"#The default value is 10. The maximum is 100.\n");
  fprintf(fp,"buffers = %i\n\n",numberOfBuffers);
  fprintf(fp,"#0 = QUIT function exits Dav\n");
  fprintf(fp,"#1 = QUIT function closes only the current buffer\n");
  fprintf(fp,"#    (If there are no more buffers left, it will exit Dav)\n");
  fprintf(fp,"bufferQuit = %i\n\n",bufferQuit);
  fprintf(fp,"#Tab width (default value is 8)\n");
  fprintf(fp,"tabWidth = %i\n\n",tabWidth);
  fprintf(fp,"#SmartCursor controls whether the cursor will try to stay on the same column\n");
  fprintf(fp,"#during up/down movement between lines of varying widths.\n");
  fprintf(fp,"smartCursor = %i\n\n",smartCursor);
  fprintf(fp,"#Optimize controls whether Dav is optimized for memory or cpu efficiency.\n");
  fprintf(fp,"#Right now this is only used for sorting directories by filename.\n");
  fprintf(fp,"#Default value is 0. If you notice lag in the load/save dialogue, you may\n");
  fprintf(fp,"# want to change it to 1.\n");
  fprintf(fp,"#0 = Optimized for memory efficiency (uses quicksort)\n");
  fprintf(fp,"#1 = Optimized for cpu efficiency (uses radix sort)\n");
  fprintf(fp,"optimize = %i\n\n",optimize);  
  fprintf(fp,"#Function Key definitions:\n");
  fprintf(fp,"#  SEARCH : Search the file for a string\n");
  fprintf(fp,"#  REPLACE : Find and replace strings\n");
  fprintf(fp,"#  SAVE : Saves current file\n");
  fprintf(fp,"#  SAVE_AS : Saves current file, prompts for filename first\n");
  fprintf(fp,"#  LOAD : Load a different file\n");
  fprintf(fp,"#  QUIT : Exits out of Dav\n");
  fprintf(fp,"#  UNDO : Undoes your last action\n");
  fprintf(fp,"#  COMPILE : Runs a Makefile and displays error messages\n");
  fprintf(fp,"#  TOGGLE_AUTOINDENT : Toggles auto indenting\n");
  fprintf(fp,"#  TOGGLE_BOTTOM_ROW : Switches between displaying Fn bindings and file name\n");
  fprintf(fp,"#  PREV : Switches to the previous text buffer (of 10)\n");
  fprintf(fp,"#  NEXT : Switches to the next text buffer (of 10)\n");
  for(t=0;t<12;t++)
  {
    fprintf(fp,"F%i = ", t+1);
    if(Fn_ptr[t] == search)
      fprintf(fp,"SEARCH\n");
    else if(Fn_ptr[t] == save)
      fprintf(fp,"SAVE\n");
    else if(Fn_ptr[t] == saveAs)
      fprintf(fp,"SAVE_AS\n");
    else if(Fn_ptr[t] == askLoad)
      fprintf(fp,"LOAD\n");
    else if(Fn_ptr[t] == tryQuit)
      fprintf(fp,"QUIT\n");
    else if(Fn_ptr[t] == Undo)
      fprintf(fp,"UNDO\n");
    else if(Fn_ptr[t] == tryCompile)
      fprintf(fp,"COMPILE\n");
    else if(Fn_ptr[t] == toggleAutoIndent)
      fprintf(fp,"TOGGLE_AUTOINDENT\n");
    else if(Fn_ptr[t] == toggleBottomRow)
      fprintf(fp,"TOGGLE_BOTTOM_ROW\n");
    else if(Fn_ptr[t] == replace)
      fprintf(fp,"REPLACE\n");
    else if(Fn_ptr[t] == goToNextBuffer)
      fprintf(fp,"NEXT\n");
    else if(Fn_ptr[t] == goToPrevBuffer)
      fprintf(fp,"PREV\n");
    else fprintf(fp,"NOTHING\n");
  }
}

void addLineAfter(struct line *whichLine, char *data)
{
  struct line *temp = whichLine->next;
  struct line *newLine = (struct line *)malloc(sizeof(struct line));
  temp->prev = newLine;
  whichLine->next = newLine;
  newLine->next = temp;
  newLine->prev = whichLine;
  newLine->data = (unsigned char *)malloc(strlen(data));
  strcpy(newLine->data, data);
  newLine->length = strlen(data);
  countTabs(newLine);
  currentBuffer->numLines++;
}

char positionDown(struct position *p) {
  /*
  Returns 0 upon normal movement
  Returns 1 if the position stayed on the same line (end of file)
  */
  
  int tempX = p->cursX;
  char temp = 0;
  
  while(!temp) {
    /* 
    Move the cursor right until it wraps to the next line.
    If it hits the end of the file, return 1 instead.
    */
    
    temp = moveRight(p);
    if(temp==1) {
      return 1;
    }
  }
  while(1) {
    /* Move the cursor right until it hits the correct location. */
    
    if(p->cursX >= tempX) {
      return 0;
    } else if(p->offset == p->l->length - 1) {
      return 0;
    }
    
    temp = moveRight(p);
    if(temp == 1) {
      return 0;
    } else if(temp > 1) {
      moveLeft(p);
      return 0;
    }
  }
}

char positionUp(struct position *p)
{
  /*
  Returns 0 upon normal movement
  Returns 1 if the position stayed on the same line (beginning of file)
  */

  int tempX = p->cursX;
  char temp=0;
  while(!temp) {
    temp = moveLeft(p);
    if(temp==1) {
      if(p == &currentBuffer->cursor) {
        currentBuffer->cursor.wantCursX = currentBuffer->cursor.cursX;
      }
      return 1;
    }
  }
  while(1) {
    if(p->cursX <= tempX) {
      return 0;
    }
    temp = moveLeft(p);
    if(temp == 1) {
      return 0;
    } else if(temp > 1) {
      moveRight(p);
      return 0;
    }
  }
}

void connectLines(struct line *baseline)
{
  struct line *l = baseline->next;
  unsigned char *ptr = (unsigned char *)malloc(baseline->length + l->length);
  memmove(ptr, baseline->data, baseline->length);
  memmove(ptr+baseline->length, l->data, l->length);
  free(baseline->data);
  baseline->data = ptr;
  baseline->length += l->length;
  baseline->next = l->next;
  l->next->prev = baseline;
  baseline->hasTabs += l->hasTabs;
  free(l->data);
  free(l);
  currentBuffer->numLines--;
}

void determineLineNum(struct position *p)
{
  struct position temp;
  int cX=p->cursX,cY=p->cursY;

  temp.l = currentBuffer->head->next;
  temp.offset = 0;
  temp.lineNum = 0;
  while(temp.l != p->l)
    positionDown(&temp);

  while(temp.offset+maxX <= p->offset)
    positionDown(&temp);

  p->lineNum = temp.lineNum;
  p->cursX=cX;
  p->cursY=cY;
}

void countTabs(struct line *l)
{
  int t;
  l->hasTabs = 0;
  for(t=l->length-1;t>=0;t--)
    if(l->data[t]==9) l->hasTabs++;
}

void determineCursX(struct position *p)
{
  struct position temp;
  unsigned char c;
  
  temp.l = p->l;
  temp.offset = 0;
  temp.cursX = 0;
  while(p->offset!=temp.offset)
  {
    c = temp.l->data[temp.offset];
    temp.offset++;
    if(c!=9)    
      temp.cursX++;
    else
      temp.cursX+= tabWidth - (temp.cursX % tabWidth); 
    if(temp.cursX >= maxX) temp.cursX=0;
  }  
  p->cursX = temp.cursX;
} 

void logMsg(char *msg) {
  char text[256];
  sprintf(text, "echo \"%s\" >> log", msg);
  system(text);
}

void sigcatch() {
  quit("");
}

void radixSort(char **strings, int number) {
  char **buckets[256];
  int numberInBuckets[256];
  int radix = 0;
  int t;
  int i,j;
  
  for(i=0; i<256; i++) {
    buckets[i] = (char **)malloc(0*sizeof(char *));
  }

  for(radix=255; radix>=0; radix--) {
    for(t=0; t<256; t++) {
      numberInBuckets[t] = 0;
    }
    
    for(t=0; t<number; t++) {
      int bucket = (int)((unsigned char)strings[t][radix]);
      buckets[bucket] = realloc(buckets[bucket], (numberInBuckets[bucket] + 1)*sizeof(char *));
      buckets[bucket][numberInBuckets[bucket]] = strings[t];
      numberInBuckets[bucket]++;
    }
    
    t = 0;
    for(i=0; i<256; i++) {
      for(j=0; j<numberInBuckets[i]; j++) {
        strings[t] = buckets[i][j];
        t++;
      }
    }
  }
  
  for(i=0; i<256; i++) {
    free(buckets[i]);
  }
}

void randomizedQuickSort(char **strings, int low, int high) {
  //Pick a random element
  int axis;
  int oldLow = low;
  int oldHigh = high;
  char *temp;
  
  if (low >= high) { //Zero or one element
    return;
  }
  
  if (high - low == 1) { //Only two elements
    if (strcmp(strings[high], strings[low]) < 0) {
      temp = strings[high];
      strings[high] = strings[low];
      strings[low] = temp;
    }
    return;
  }
  
  //Swap axis element with last element
  axis = (rand() % (high - low + 1)) + low;
  temp = strings[axis];
  strings[axis] = strings[high];
  strings[high] = temp;
  axis = high;
  high--;
  
  while (1) {
    while (strcmp(strings[low], strings[axis]) <= 0 && low < high) {
      low++;
    }
    
    while (strcmp(strings[high], strings[axis]) >= 0 && low < high) {
      high--;
    }
    
    if (low >= high) {
      break;
    }
    
    temp = strings[low];
    strings[low] = strings[high];
    strings[high] = temp;
  }

  if (strcmp(strings[low], strings[axis]) < 0) {
    low++;
  }  
  temp = strings[axis];
  strings[axis] = strings[low];
  strings[low] = temp;
  
  randomizedQuickSort(strings, oldLow, low-1);
  randomizedQuickSort(strings, low+1, oldHigh);
}
