#include <stdlib.h>
#include <fcntl.h>
#include <sndfile.h>
#include <pthread.h>
// #include <semaphore.h>
#include "hideShow.h"
#include "tools.h"
#include "controlSong.h"

#define FRAMES_PER_BUFFER 64
// #define FILE_NAME "./audios/audio2-16bits.wav"
#define MAX_FILES 99    // Maxima cantidad de archivos wav que leera scanFolder
#define LENGTH_FILES 50 // Tamanio maximo en el nombre de los archivos wav

// Estructura para almacenar los metadatos del archivo
typedef struct
{
    char title[20];
    char artist[20];
    int year;
    int songLength;
    int fileSize;
    char album[15];
    char genre[15];
    char fileName[LENGTH_FILES];
    const char *directorio;
} Song;

typedef struct
{
    PaStream *stream;
    PaError err;
    AudioData audioData;            // Estructura que guarda los datos de los audios, cabezera y variables de control
    const PaDeviceInfo *deviceInfo; // Estructura para pasar la latencia
    PaStreamParameters outputParams;
    Song songs[MAX_FILES];
    int bitsPerSample;
    int numFiles; // Guardara el numero de archivos wav leidos por la funcion scanFolder
    char **fileNames;
    const char *directorio;
} DatosHiloPlayer;

// Funcion del hilo para contar los segundos
void *contarSegundos(void *arg)
{
    AudioData *datos = (AudioData *)arg;

    while (1)
    {
        if (datos->isEnd)
            break;
        if (!datos->isPaused)
        {
            usleep(10000);
            datos->currentTime += 0.01; // Incrementar los segundos
        }
    }
    pthread_exit(NULL);
}

// Funcion del hilo para contar los segundos
void *player(void *arg)
{

    pthread_exit(NULL);
}

// Función de callback para el procesamiento de audio de punto flotante de 32 bits
int audioFloat32Callback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData)
{
    AudioData *audioData = (AudioData *)userData;
    float *out = (float *)outputBuffer;

    // Leer los datos del archivo de audio y los copiamos al bufer de salida
    size_t bytesRead = fread(out, sizeof(float), framesPerBuffer * audioData->numChannels, audioData->file);

    if (bytesRead < framesPerBuffer * audioData->numChannels)
    {
        // Si se llega al final del archivo, se detiene la reproducción
        audioData->isEnd = true;
        return paComplete;
    }
    return paContinue;
}

// Función de callback para el procesamiento de audio entero de 16 bits
int audioInt16Callback(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData)
{
    AudioData *audioData = (AudioData *)userData;
    short *out = (short *)outputBuffer;

    // Leer los datos del archivo de audio
    size_t bytesRead = fread(out, sizeof(short), framesPerBuffer * audioData->numChannels, audioData->file);

    if (bytesRead < framesPerBuffer * audioData->numChannels)
    {
        // Si se llega al final del archivo, se detiene la reproduccion
        audioData->isEnd = true;
        return paComplete;
    }
    return paContinue;
}

int main()
{
    // PaStream *stream;
    // PaError err;
    // AudioData audioData;            // Estructura que guarda los datos de los audios, cabezera y variables de control
    // const PaDeviceInfo *deviceInfo; // Estructura para pasar la latencia
    // PaStreamParameters outputParams;
    // Song songs[MAX_FILES];
    // int bitsPerSample;
    pthread_t contador;
    DatosHiloPlayer datos;
    datos.songs->directorio = "audios/"; // Ruta del directorio a explorar
    // int numFiles;                       // Guardara el numero de archivos wav leidos por la funcion scanFolder
    // char **fileNames;

    datos.fileNames = loadSongsFromDirectoty(datos.songs->directorio, &(datos.numFiles), MAX_FILES, LENGTH_FILES); // Retorna una matriz con los nombres de los archivos wav

    if (datos.fileNames == NULL)
    {
        return 1;
    }
    if (datos.numFiles == 0)
        printf("\nNo hay archivos wav en el directorio\n");
    else if (datos.numFiles > 100)
        printf("Supera el maximo de 100 canciones permitidas");
    {
        // Copia los nombres de los archivos a la estructura Songs
        for (int i = 0; i < datos.numFiles; i++)
        {
            strcpy(datos.songs[i].fileName, datos.fileNames[i]);
        }

        // Inicializar la biblioteca PortAudio
        datos.err = Pa_Initialize();
        system("clear");
        if (datos.err != paNoError)
        {
            printf("Error al inicializar PortAudio: %s\n", Pa_GetErrorText(datos.err));
            return 1;
        }

        // Selecciona la cancion y se reproduce
        datos.audioData.file = printSongs(datos.fileNames, datos.numFiles, datos.songs->directorio);

        // Leer la cabecera del archivo WAV
        char header[44];
        fread(header, sizeof(char), 44, datos.audioData.file);

        // Obtener el numero de canales
        fseek(datos.audioData.file, 22, SEEK_SET);
        fread(&(datos.audioData).numChannels, sizeof(short), 1, datos.audioData.file);

        // Obtener la velocidad de muestreo del archivo de audio
        fseek(datos.audioData.file, 24, SEEK_SET);
        fread(&(datos.audioData.sampleRate), sizeof(int), 1, datos.audioData.file);

        // Obtener información del dispositivo de salida
        datos.outputParams.device = Pa_GetDefaultOutputDevice(); // Obtenemos el identificador del dispositivo de salida por defecto
        datos.deviceInfo = Pa_GetDeviceInfo(datos.outputParams.device);
        datos.outputParams.channelCount = datos.audioData.numChannels;                   // Establecemos el numero de canales
        datos.outputParams.suggestedLatency = datos.deviceInfo->defaultLowOutputLatency; // Se establece la latencia
        datos.outputParams.hostApiSpecificStreamInfo = NULL;

        // Verificar el tamaño de muestra
        datos.bitsPerSample = header[34] + (header[35] << 8); // Obtener los bits por muestra desde la cabecera

        // 1.Establece los parametros dependiendo de si es de punto flotante de 32 bits o entero de 16 bits
        // 2.Abre el flujo de audio
        if (datos.bitsPerSample == 32)
        {
            datos.outputParams.sampleFormat = paFloat32; // Se establece el formato de muestreo
            // Preparación de la reproduccion
            fseek(datos.audioData.file, 34, SEEK_SET);
            // Abrir el flujo de salida de audio
            datos.err = Pa_OpenStream(&(datos.stream), NULL, &(datos.outputParams), datos.audioData.sampleRate,
                                FRAMES_PER_BUFFER, paNoFlag, audioFloat32Callback, &(datos.audioData));
        }
        else if (datos.bitsPerSample == 16)
        {
            datos.outputParams.sampleFormat = paInt16; // Se establece el formato de muestreo
            // Preparación de la reproduccion
            fseek(datos.audioData.file, 44, SEEK_SET);
            // Abrir el flujo de salida de audio
            datos.err = Pa_OpenStream(&(datos.stream), NULL, &(datos.outputParams), datos.audioData.sampleRate,
                                FRAMES_PER_BUFFER, paNoFlag, audioInt16Callback, &(datos.audioData));
        }
        else
        {
            printf("El archivo WAV no contiene datos de punto flotante de 32 bits ni enteros de 16 bits.\n");
        }

        if (datos.err != paNoError)
        {
            printf("Error al abrir el flujo de audio: %s\n", Pa_GetErrorText(datos.err));
            fclose(datos.audioData.file);
            Pa_Terminate();
            return 1;
        }

        // Iniciar la reproducción
        datos.err = Pa_StartStream(datos.stream);
        if (datos.err != paNoError)
        {
            printf("Error al iniciar la reproducción: %s\n", Pa_GetErrorText(datos.err));
            fclose(datos.audioData.file);
            Pa_CloseStream(datos.stream);
            Pa_Terminate();
            return 1;
        }

        // Inicio el contador de segundos en 0
        datos.audioData.currentTime = 0;

        // Crear el hilo contador de segundos reproducidos
        if (pthread_create(&contador, NULL, contarSegundos, (void *)&(datos.audioData)) != 0)
        {
            fprintf(stderr, "Error al crear el hilo\n");
            return 1;
        }

        // Funcion que controla la pausa, el stop, la cancion siguiente y la anterior
        stopPlayNextBack(&(datos.audioData), datos.stream);

        // Detener la reproducción
        datos.err = Pa_StopStream(datos.stream);
        if (datos.err != paNoError)
        {
            // printf("Error al detener la reproducción: %s\n", Pa_GetErrorText(err));
        }

        // Cerrar el flujo de audio
        datos.err = Pa_CloseStream(datos.stream);
        if (datos.err != paNoError)
        {
            printf("Error al cerrar el flujo de audio: %s\n", Pa_GetErrorText(datos.err));
        }

        // Terminar PortAudio
        Pa_Terminate();

        // Cerrar el archivo de audio
        fclose(datos.audioData.file);
        position(5, 10);
        printf("Reproducción detenida.\n");
        pthread_join(contador, NULL);
        showCursor();
        restoreInputBuffer();
    }
    freeFileNames(datos.fileNames, datos.numFiles);

    return 0;
}