#include <stdlib.h>
#include <fcntl.h>
#include <sndfile.h>
#include <pthread.h>
// #include <semaphore.h>
#include "hideShow.h"
#include "tools.h"
#include "controlSong.h"

#define FRAMES_PER_BUFFER 64 // Tamanio del buffer
#define MAX_FILES 99         // Maxima cantidad de archivos wav que leera scanFolder
#define LENGTH_FILES 50      // Tamanio maximo en el nombre de los archivos wav

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

// Estructura para almacenar los datos que recibira el hilo player
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
    bool error; // Sera true si hay un error en el hilo
} DataThread;

// Función de callback para el procesamiento de audio de punto flotante de 32 bits
int audioFloat32Callback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData);

// Función de callback para el procesamiento de audio entero de 16 bits
int audioInt16Callback(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData);

// Funcion para establecer los parametros necesarios para la reproduccion y abrir el flujo de audio
void configureAudio(DataThread *dataThread);

// Funcion del hilo para contar los segundos
void *countSeconds(void *arg);

// Funcion del hilo para reproducir el sonido
void *player(void *arg);

// Funcionn para seleccionar un audio
void *selector(void *arg);

int main()
{
    pthread_t tContador, tPlayer;
    DataThread dataThread;
    dataThread.songs->directorio = "./"; // Ruta del directorio a explorar
    dataThread.error = false;

    // Crear el hilo player
    if (pthread_create(&tPlayer, NULL, player, (void *)&dataThread) != 0)
    {
        showCursor();
        restoreInputBuffer();
        printf("\nError al crear el hilo player");
        return 1;
    }

    // Crear el hilo contador de segundos reproducidos
    if (pthread_create(&tContador, NULL, countSeconds, (void *)&(dataThread.audioData)) != 0)
    {
        showCursor();
        restoreInputBuffer();
        printf("\nError al crear el hilo contador");
        return 1;
    }

    // Esperar a que el hilo tPlayer finalice
    pthread_join(tPlayer, NULL);
    if (dataThread.error == true)
    {
        showCursor();
        restoreInputBuffer();
        printf("\nError en el hilo tPlayer\n");
        exit(1);
    }
    // Esperar a que el hilo tContador finalice
    pthread_join(tContador, NULL);

    showCursor();
    restoreInputBuffer();
    freeFileNames(dataThread.fileNames, dataThread.numFiles);

    return 0;
}

int audioFloat32Callback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData)
{
    AudioData *audioData = (AudioData *)userData;
    float *out = (float *)outputBuffer;

    // Leer los datos del archivo de audio y los copiamos al bufer de salida
    size_t bytesRead = fread(out, sizeof(float), framesPerBuffer * audioData->numChannels,
                             audioData->file);

    if (bytesRead < framesPerBuffer * audioData->numChannels)
    {
        // Si se llega al final del archivo, se detiene la reproducción
        audioData->indicator = false; // Indicador de seleccion
        audioData->isPlaying = false;
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
    size_t bytesRead = fread(out, sizeof(short), framesPerBuffer * audioData->numChannels,
                             audioData->file);

    if (bytesRead < framesPerBuffer * audioData->numChannels)
    {
        // Si se llega al final del archivo, se detiene la reproduccion
        audioData->indicator = false;
        audioData->isPlaying = false;
        return paComplete;
    }
    return paContinue;
}

// Funcion para establecer los parametros necesarios para la reproduccion y abrir el flujo de audio
void configureAudio(DataThread *dataThread)
{
    // Leer la cabecera del archivo WAV
    char header[44];
    fread(header, sizeof(char), 44, dataThread->audioData.file);

    // Obtener el numero de canales
    fseek(dataThread->audioData.file, 22, SEEK_SET);
    fread(&(dataThread->audioData).numChannels, sizeof(short), 1, dataThread->audioData.file);

    // Obtener la velocidad de muestreo del archivo de audio
    fseek(dataThread->audioData.file, 24, SEEK_SET);
    fread(&(dataThread->audioData.sampleRate), sizeof(int), 1, dataThread->audioData.file);

    // Obtener información del dispositivo de salida
    dataThread->outputParams.device = Pa_GetDefaultOutputDevice(); // Obtenemos el identificador del dispositivo de salida por defecto
    dataThread->deviceInfo = Pa_GetDeviceInfo(dataThread->outputParams.device);
    dataThread->outputParams.channelCount = dataThread->audioData.numChannels;                   // Establecemos el numero de canales
    dataThread->outputParams.suggestedLatency = dataThread->deviceInfo->defaultLowOutputLatency; // Se establece la latencia
    dataThread->outputParams.hostApiSpecificStreamInfo = NULL;

    // Verificar el tamaño de muestra
    dataThread->bitsPerSample = header[34] + (header[35] << 8); // Obtener los bits por muestra desde la cabecera

    // 1.Establece los parametros dependiendo de si es de punto flotante de 32 bits o entero de 16 bits
    // 2.Abre el flujo de audio
    if (dataThread->bitsPerSample == 32)
    {
        dataThread->outputParams.sampleFormat = paFloat32; // Se establece el formato de muestreo
        // Preparación de la reproduccion
        fseek(dataThread->audioData.file, 34, SEEK_SET);
        // Abrir el flujo de salida de audio
        dataThread->err = Pa_OpenStream(&(dataThread->stream), NULL, &(dataThread->outputParams), dataThread->audioData.sampleRate,
                                        FRAMES_PER_BUFFER, paNoFlag, audioFloat32Callback, &(dataThread->audioData));
    }
    else if (dataThread->bitsPerSample == 16)
    {
        dataThread->outputParams.sampleFormat = paInt16; // Se establece el formato de muestreo
        // Preparación de la reproduccion
        fseek(dataThread->audioData.file, 44, SEEK_SET);
        // Abrir el flujo de salida de audio
        dataThread->err = Pa_OpenStream(&(dataThread->stream), NULL, &(dataThread->outputParams), dataThread->audioData.sampleRate,
                                        FRAMES_PER_BUFFER, paNoFlag, audioInt16Callback, &(dataThread->audioData));
    }
    else
    {
        printf("El archivo WAV no contiene datos de punto flotante de 32 bits ni enteros de 16 bits.\n");
    }
    // El indicador de seleccion se desactiva
    dataThread->audioData.indicator = false;
    if (dataThread->err != paNoError)
    {
        printf("Error al abrir el flujo de audio: %s\n", Pa_GetErrorText(dataThread->err));
        fclose(dataThread->audioData.file);
        Pa_Terminate();
        dataThread->error = true;
        pthread_exit(NULL);
    }

    // Iniciar la reproducción
    dataThread->err = Pa_StartStream(dataThread->stream);
    if (dataThread->err != paNoError)
    {
        printf("Error al iniciar la reproducción: %s\n", Pa_GetErrorText(dataThread->err));
        fclose(dataThread->audioData.file);
        Pa_CloseStream(dataThread->stream);
        Pa_Terminate();
        dataThread->error = true;
        pthread_exit(NULL);
    }
}

void *countSeconds(void *arg)
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
}

void *player(void *arg)
{
    pthread_t tSelector;
    DataThread *dataThread = (DataThread *)arg; // Casteamos y recuperamos la informacion pasada al hilo

    // Retorna una matriz con los nombres de los archivos wav y establece el numero de archivos en numFiles
    dataThread->fileNames = loadSongsFromDirectoty(dataThread->songs->directorio,
                                                   &(dataThread->numFiles), MAX_FILES,
                                                   LENGTH_FILES, &(dataThread->error));
    if (dataThread->fileNames == NULL) // No se pueden recuperar los nombres de los archivos wav
    {
        dataThread->error = true;
        pthread_exit(NULL);
    }
    if (dataThread->numFiles == 0)
        printf("\nNo hay archivos wav en el directorio\n");
    else if (dataThread->numFiles > 100)
        printf("Supera el maximo de 100 canciones permitidas");
    {
        // Copia los nombres de los archivos a la estructura Songs
        for (int i = 0; i < dataThread->numFiles; i++)
        {
            strcpy(dataThread->songs[i].fileName, dataThread->fileNames[i]);
        }

        // Inicializar la biblioteca PortAudio
        dataThread->err = Pa_Initialize();
        system("clear");
        if (dataThread->err != paNoError)
        {
            printf("Error al inicializar PortAudio: %s\n", Pa_GetErrorText(dataThread->err));
            dataThread->error = true;
            pthread_exit(NULL);
        }

        // Se crea el hilo que selecciona la cancion
        pthread_create(&tSelector, NULL, selector, (void *)dataThread);
        dataThread->audioData.indicator = false;

        // Esperar a que el hilo tSelector finalice
        pthread_join(tSelector, NULL);

        // Terminar PortAudio
        Pa_Terminate();
        // Cerrar el archivo de audio
        if (dataThread->audioData.file != NULL)
        { // Si se llego a seleccionar un audio se cerrara
            fclose(dataThread->audioData.file);
        }

        position(X_SELECT - 10, Y_SELECT + 7);
        printf("                ");
        position(X_SELECT - 10, Y_SELECT + 7);
        printf("Reproducción detenida.\n");
    }
}

void *selector(void *arg)
{
    DataThread *dataThread = (DataThread *)arg; // Casteamos y recuperamos la informacion pasada al hilo
    dataThread->audioData.select = 0;           // Inicialmente se seleccionara el audio 0
    dataThread->audioData.indicator = true;     // El audio no iniciara al abrir el programa y sin hacer una seleccion
    dataThread->audioData.file = NULL;          // Se inciliza el apuntador
    while (true)
    {
        // Se establece el tiempo de reproduccion en 0
        dataThread->audioData.currentTime = 0;

        // Selecciona la cancion, abre el archivo y regresa el archivo a reproducir
        printSongs(dataThread->fileNames, dataThread->numFiles,
                   dataThread->songs->directorio, &(dataThread->audioData.select),
                   &(dataThread->audioData.file), &(dataThread->audioData), dataThread->stream);

        if (dataThread->audioData.isEnd == true) // Si se manda una orden de finalizacion del programa, sale del bucle
            break;

        // Configura los parametros para la reproduccion de audio, abre el flujo de audio y comienza la reproduccion
        configureAudio(dataThread); // Utiliza internamente dataThread->audioData.file para saber que parametros configurar
    }
}