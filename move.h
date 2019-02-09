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
#ifndef move_h
#define move_h

#include "structs.h"

char positionDown(struct position *p);
char positionUp(struct position *p);
void moveUp(struct position *p);
void moveDown(struct position *p);
char moveLeft(struct position *p); //Returns 1 when you hit the beginning of the buffer
char moveRight(struct position *p); //Returns 1 when you hit the end of the buffer
char scrollDown();
char scrollUp();

#endif
