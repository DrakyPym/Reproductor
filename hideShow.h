#ifndef HIDESHOW_H
#define HIDESHOW_H

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

void hideCursor();
void showCursor();
void hideInputBuffer();
void restoreInputBuffer();

#endif