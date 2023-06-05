#ifndef CONTROL_SONG_H
#define CONTROL_SONG_H
#include <stdbool.h>
#include <portaudio.h>
#include <string.h>
#include "tools.h"
#include "hideShow.h"

#define X_SELECT 15 // Variable para modificar la posicion del menu
#define Y_SELECT 7 //Variable para modificar la posicion del menu

// Definir colores de escape ANSI
#define RESET_COLOR   "\x1B[0m"
#define RED_COLOR     "\x1B[31m"

// Estructura que guarda los datos de los audios, cabezera y variables de control
typedef struct
{
    FILE *file;
    int sampleRate;
    int numChannels;
    long int format;
    bool isPaused;
    bool isEnd;
    float currentTime;
} AudioData;

void stopPlayNextBackTime(AudioData *data, PaStream *stream);
void printSongs(char **fileNames, int numFiles, const char *directorio, int *select, FILE **file1, AudioData *data);
#endif