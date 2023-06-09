#ifndef TOOLS_H
#define TOOLS_H

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <sndfile.h>


#define MAX_PATH 4096 // Valor máximo para la ruta

char getkey();
void position(int x, int y);
char **loadSongsFromDirectoty(const char *directorio, int *numFiles, int maxFiles, int length, bool *error);
void freeFileNames(char **fileNames, int numFiles);
//void data(char *title, char *artis, int *year, int *songLenght, int *filSize, char *album, char genre, const char *directorio, char **fileNames);

#endif