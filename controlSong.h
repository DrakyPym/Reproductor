#ifndef CONTROL_SONG_H
#define CONTROL_SONG_H
#include <stdbool.h>
#include <portaudio.h>
#include <string.h>
#include <pthread.h>
#include "tools.h"
#include "hideShow.h"

#define X_SELECT 15 // Variable para modificar la posicion del menu
#define Y_SELECT 5 //Variable para modificar la posicion del menu

// Definir colores de escape ANSI
#define RESET_COLOR   "\x1B[0m"
#define RED_COLOR     "\x1B[31m"

// Estructura que guarda los datos de los audios, cabezera y variables de control
typedef struct
{
    FILE *file;
    int sampleRate;    
    int numChannels;
    long int format; //Guarda el formato
    bool isPaused; //Indica si el audio ha sido pausado
    bool isEnd; // Indica si el programa ha finalizado
    float currentTime; //Guarda el tiempo que lleva reproducido
    bool isPlaying;
    bool indicator; //Indica si se selecciono una cancion
    int select; // Inicialmente el audio seleccionado
} AudioData;
void stopAudio(AudioData *data, PaStream *stream);

//void stopPlayNextBackTime(AudioData *data, PaStream *stream);
void printSongs(char **fileNames, int numFiles, const char *directorio, int *select, FILE **file1, AudioData *data, PaStream *stream);

#endif