#include "controlSong.h"

void stopPlayNextBack(AudioData *data, PaStream *stream)
{
    int quit = 0;
    printf("Reproduciendo audio. Presiona 'p' para pausar o 'q' para salir.\n");

    while (!quit)
    {
        fflush(stdin);
        if (data->isEnd)
            break;
        hideCursor();
        position(5, 10);
        printf("Tiempo: %i segundos", (int)data->currentTime);
        usleep(10000);
        hideInputBuffer();
        char key = getkey();
        if (key != '\0')
        {
            switch (key)
            {
            case 'p':
                fflush(stdin);
                data->isPaused = !data->isPaused;
                if (data->isPaused)
                {
                    Pa_StopStream(stream);
                    position(5, 11);
                    printf("Audio en pausa.\n");
                }
                else
                {
                    Pa_StartStream(stream);
                    position(5, 11);
                    printf("Audio reanudado.\n");
                }
                break;
            case 'q':
                fflush(stdin);
                quit = 1;
                data->isEnd = true;
                break;
            }
        }
    }
}

FILE *printSongs(char **fileNames, int numFiles, const char *directorio)
{
    int quit = 0;
    int select = 0; // Indica que numero de cancion se seleccionara
    int i;
    char fileOpen[100]; // Va a guardar la ruta completa del archivo de audio a reproducir
    FILE *file;

    position(X_SELECT, Y_SELECT);
    printf("Seleccione una cancion");
    for (i = 0; i < numFiles; i++)
    {
        position(X_SELECT, Y_SELECT + i + 1);
        printf("%i. %s", i + 1, fileNames[i]); // Se imprimen los nombres de las canciones
    }
    position(X_SELECT, Y_SELECT + 1);
    printf(RED_COLOR);
    printf("1. %s", fileNames[select]);
    while (!quit)
    {
        fflush(stdin);
        usleep(10000);
        hideCursor();
        hideInputBuffer();
        char key = getkey();
        if (key != '\0')
        {
            switch (key)
            {
            case 'U': // Flecha arriba
                if (select == 0)
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                    select = numFiles - 1;
                    position(X_SELECT, Y_SELECT + select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                }
                else
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + select + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                    select--;
                    position(X_SELECT, Y_SELECT + select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                }
                break;
            case 'D': // Flecha abajo
                if (select == numFiles - 1)
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + select + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                    select = 0;
                    position(X_SELECT, Y_SELECT + select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                }
                else
                {
                    fflush(stdin);
                    position(X_SELECT, Y_SELECT + select + 1);
                    printf(RESET_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                    select++;
                    position(X_SELECT, Y_SELECT + select + 1);
                    printf(RED_COLOR);
                    printf("%i. %s", select + 1, fileNames[select]);
                }
                break;
            case 'E': // Enter
                printf(RESET_COLOR);
                fflush(stdin);
                system("clear");
                quit = 1;
                break;
            }
        }
    }

    strcpy(fileOpen, directorio);
    strcat(fileOpen, fileNames[select]);

    file = fopen(fileOpen, "rb");
    if (!file)
    {
        // printf("No se pudo abrir el archivo de audio.\n");
        Pa_Terminate();
    }

    return file;
}