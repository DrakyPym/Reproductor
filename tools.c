#include "tools.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

void position(int x, int y)
{
    printf("\033[%d;%dH", y, x);
}

char getkey()
{
    char key = '\0';

    struct termios oldTermios, newTermios;
    tcgetattr(STDIN_FILENO, &oldTermios);
    newTermios = oldTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);

    int oldFileFlags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldFileFlags | O_NONBLOCK);

    int ch = getchar();
    if (ch != EOF)
    {
        key = (char)ch;

        // Verificar si es la tecla Enter
        if (key == '\n')
        {
            key = 'E'; // Utilizamos 'E' como representación de la tecla Enter
        }
        else if (key == '\033')
        {
            // Leer el siguiente carácter para determinar la tecla especial
            key = getchar();

            if (key == '[')
            {
                // Leer el siguiente carácter para identificar la tecla específica
                key = getchar();

                switch (key)
                {
                case 'A':      // Tecla de flecha hacia arriba
                    key = 'U'; // Utilizamos 'U' como representación de la flecha hacia arriba
                    break;
                case 'B':      // Tecla de flecha hacia abajo
                    key = 'D'; // Utilizamos 'D' como representación de la flecha hacia abajo
                    break;
                case 'C':      // Tecla de flecha hacia la derecha
                    key = 'R'; // Utilizamos 'R' como representación de la flecha hacia la derecha
                    break;
                case 'D':      // Tecla de flecha hacia la izquierda
                    key = 'L'; // Utilizamos 'L' como representación de la flecha hacia la izquierda
                    break;
                default:
                    break;
                }
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
    fcntl(STDIN_FILENO, F_SETFL, oldFileFlags);

    return key;
}

char **loadSongsFromDirectoty(const char *directorio, int *numFiles, int maxFiles, int length)
{
    DIR *dir;
    struct dirent *entry;
    char rutaCompleta[MAX_PATH];

    dir = opendir(directorio); // Abrir el directorio
    if (dir == NULL)
    {
        perror("Error al abrir el directorio"); // Mostrar error si no se puede abrir
        return NULL;
    }

    char **fileNames = malloc(maxFiles * sizeof(char *));
    if (fileNames == NULL)
    {
        perror("Error de memoria"); // Mostrar error si no se puede asignar memoria
        closedir(dir);
        return NULL;
    }
    for (int i = 0; i < maxFiles; i++)
    {
        fileNames[i] = malloc((length + 1) * sizeof(char));
    }

    int count = 0;
    while ((entry = readdir(dir)) != NULL && count < maxFiles)
    { // Iterar sobre las entradas del directorio
        struct stat archivoStat;

        snprintf(rutaCompleta, MAX_PATH, "%s/%s", directorio, entry->d_name); // Construir la ruta completa del archivo

        if (stat(rutaCompleta, &archivoStat) == 0)
        { // Obtener información del archivo
            if (S_ISREG(archivoStat.st_mode))
            {                                                         // Verificar si es un archivo regular
                const char *extension = ".wav";                       // Extensión deseada
                size_t longitudExtension = strlen(extension);         // Longitud de la extensión
                size_t longitudNombreArchivo = strlen(entry->d_name); // Longitud del nombre del archivo

                if (longitudNombreArchivo >= longitudExtension &&
                    strcmp(entry->d_name + longitudNombreArchivo - longitudExtension, extension) == 0)
                {
                    fileNames[count] = malloc((longitudNombreArchivo + 1) * sizeof(char));
                    if (fileNames[count] == NULL)
                    {
                        perror("Error de memoria"); // Mostrar error si no se puede asignar memoria
                        closedir(dir);
                        for (int i = 0; i < count; i++)
                        {
                            free(fileNames[i]);
                        }
                        free(fileNames);
                        return NULL;
                    }

                    strcpy(fileNames[count], entry->d_name);
                    count++;
                }
            }
        }
    }

    closedir(dir); // Cerrar el directorio
    *numFiles = count;
    return fileNames; // Devuelve la matriz de nombres de archivo
}

void freeFileNames(char **fileNames, int numFiles)
{
    for (int i = 0; i < numFiles; i++)
    {
        free(fileNames[i]);
    }
    free(fileNames);
}