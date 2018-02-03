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
#ifndef screenIO_h
#define screenIO_h

#include "structs.h"

void displayScreen();
void showRow();
void helpBar();
void displayBottomRow();
void displayLine(int line, struct position *pos);

/* 
 * do not modify the whitespace in the defines
 * the whitespace is used for padding the
 * bottombar in the editor (helpbar).
 */
#define KEY_F1       "F1:"
#define KEY_F2       "F2:"
#define KEY_F3       "F3:"
#define KEY_F4       "F4:"
#define KEY_F5       "F5:"
#define KEY_F6       "F6:"
#define KEY_F8       "F8:"
#define KEY_F9       "F9:"
#define KEY_F10      "F10:"

#define KEY_F1_TEXT  "Search | "
#define KEY_F2_TEXT  "Save File | "
#define KEY_F3_TEXT  "Save As | "
#define KEY_F4_TEXT  "Load File | "
#define KEY_F5_TEXT  "Quit | "
#define KEY_F6_TEXT  "Undo Change | "
#define KEY_F8_TEXT  "Auto Indent | "
#define KEY_F9_TEXT  "Compile | "
#define KEY_F10_TEXT "Bottom Row |"

#endif
