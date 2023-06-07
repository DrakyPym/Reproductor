#include "controlSong.h"

// Funcion para detener la reproduccion y cerrar el flujo de audio
void stopAudio(AudioData *data, PaStream *stream)
{
    PaError err;
    // Detener la reproducción
    err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        pthread_exit(NULL);
    }

    // Cerrar el flujo de audio
    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        printf("Error al cerrar el flujo de audio: %s\n", Pa_GetErrorText(err));
    }
}

void printSongs(char **fileNames, int numFiles, const char *directorio, int *select, FILE **file1, AudioData *data, PaStream *stream)
{
    bool quit = false;
    int i;
    char fileOpen[100]; // Va a guardar la ruta completa del archivo de audio a reproducir
    FILE *file;
    hideCursor();
    hideInputBuffer();
    position(X_SELECT, Y_SELECT);
    printf("Seleccione una cancion");
    for (i = 0; i < numFiles; i++)
    {
        position(X_SELECT, Y_SELECT + i + 1);
        printf("%i. %s", i + 1, fileNames[i]); // Se imprimen los nombres de las canciones
    }
    position(X_SELECT, Y_SELECT + *select + 1);
    printf(RED_COLOR);
    printf("%i. %s", *select + 1, fileNames[*select]);
    while (!quit)
    {
        usleep(10000); // Aligera la carga del procesador
        if (data->isPlaying)
        {
            position(5, 2);
            printf("Reproduciendo audio. Presiona 'p' para pausar o 'q' para salir.\n");
            position(5, 3);
            printf("Tiempo: %i segundos", (int)data->currentTime);
        }
        fflush(stdin);
        hideCursor();
        hideInputBuffer();
        char key = getkey();
        if (key != '\0')
        {
            switch (key)
            {
            case 'U': // Flecha arriba
                if (*select == 0)
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                    (*select) = numFiles - 1;
                    position(X_SELECT, Y_SELECT + *select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                }
                else
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + *select + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                    (*select)--;
                    position(X_SELECT, Y_SELECT + *select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                }
                break;
            case 'D': // Flecha abajo
                if (*select == numFiles - 1)
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + *select + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                    (*select) = 0;
                    position(X_SELECT, Y_SELECT + *select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                }
                else
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + *select + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                    (*select)++;
                    position(X_SELECT, Y_SELECT + *select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", *select + 1, fileNames[*select]);
                }
                break;
            case 'E': // Enter
                quit = true;
                if(data->isPaused){
                    Pa_StartStream(stream);
                }
                data->isPaused = false;
                printf(RESET_COLOR);
                fflush(stdin);
                system("clear");
                strcpy(fileOpen, directorio);
                strcat(fileOpen, fileNames[*select]);
                // Si ya existe un audio reproduciendo, cierra el flujo de audio
                if (data->isPlaying == true){
                    stopAudio(data, stream);
                    data->isPlaying = false;
                }
                file = fopen(fileOpen, "rb");
                if (!file)
                {
                    // Terminar portAudio
                    Pa_Terminate();
                }
                data->isPlaying = true;
                (*file1) = file;
                break;
            case 'p': // Letra p de pausa
                fflush(stdin);
                data->isPaused = !data->isPaused;
                if (data->isPaused)
                {
                    Pa_StopStream(stream);
                    position(5, 11);
                    printf("Audio en pausa. \n");
                }
                else
                {
                    Pa_StartStream(stream);
                    position(5, 11);
                    printf("Audio reanudado.\n");
                }
                break;
            case 'q': // Letra q de quitar
                fflush(stdin);
                quit = true;
                data->isEnd = true;
                break;
            }
        }
    }
}

