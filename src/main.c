#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <portaudio.h>

#ifdef WIN32
#include <Windows.h>
#ifdef PA_USE_ASIO
#include <pa_asio.h>
#endif
#endif

#define SAMPLE_RATE       (44100)
#define FRAMES_PER_BUFFER   (256)
/* #define DITHER_FLAG     (paDitherOff)  */
#define DITHER_FLAG           (0)

#if 1
#define PA_SAMPLE_TYPE  paFloat32
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 0
#define PA_SAMPLE_TYPE  paInt16
#define SAMPLE_SIZE (2)
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt24
#define SAMPLE_SIZE (3)
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
#define SAMPLE_SIZE (1)
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
#define SAMPLE_SIZE (1)
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

static bool running = true;

static void PrintSupportedStandardSampleRates(
    const PaStreamParameters* inputParameters,
    const PaStreamParameters* outputParameters)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int     i, printCount;
    PaError err;

    printCount = 0;
    for (i = 0; standardSampleRates[i] > 0; i++)
    {
        err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
        if (err == paFormatIsSupported)
        {
            if (printCount == 0)
            {
                printf("\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            }
            else if (printCount == 4)
            {
                printf(",\n\t%8.2f", standardSampleRates[i]);
                printCount = 1;
            }
            else
            {
                printf(", %8.2f", standardSampleRates[i]);
                ++printCount;
            }
        }
    }
    if (!printCount)
        printf("None\n");
    else
        printf("\n");
}

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        running = false;
    }
}

int request_high_priority()
{
#ifdef WIN32
    DWORD dw_err;
    DWORD dw_priclass;

    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
    {
        dw_err = GetLastError();
        if (dw_err)
        {
            printf("couldn't get realtime priority\n");
            return 1;
        }
    }

    dw_priclass = GetPriorityClass(GetCurrentProcess());
    if (dw_priclass != REALTIME_PRIORITY_CLASS)
    {
        printf("couldn't get realtime priority\n");
        return 1;
    }

    printf("set priority to 0x%x\n", dw_priclass);

#endif
#ifdef __linux__


#endif
    return 0;
}


PaError init_audio(int* n_devices)
{
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError)
    {
        return err;
    }

    *n_devices = Pa_GetDeviceCount();
    if (*n_devices < 0)
    {
        return *n_devices;
    }

    return 0;
}

void shutdown_audio()
{
    Pa_Terminate();
}

void list_devices(int n_devices, bool extra_info)
{
    const PaDeviceInfo* dev_info;
    PaStreamParameters in_params;
    PaStreamParameters out_params;
    PaError err;

    for (int i = 0; i < n_devices; ++i)
    {
        dev_info = Pa_GetDeviceInfo(i);
        printf("--------------------------------------- device #%d\n", i);

        if (i == Pa_GetDefaultInputDevice())
        {

        }
        else if (i == Pa_GetHostApiInfo(dev_info->hostApi)->defaultInputDevice)
        {

        }

        if (i == Pa_GetDefaultOutputDevice())
        {

        }
        else if (i == Pa_GetHostApiInfo(dev_info->hostApi)->defaultOutputDevice)
        {

        }

#ifdef WIN32
        {   /* Use wide char on windows, so we can show UTF-8 encoded device names */
            wchar_t wideName[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, dev_info->name, -1, wideName, MAX_PATH - 1);
            wprintf(L"Name                        = %s\n", wideName);
        }
#else
        printf("Name                        = %s\n", dev_info->name);
#endif
        printf("Host API                    = %s\n", Pa_GetHostApiInfo(dev_info->hostApi)->name);
        printf("Max inputs = %d", dev_info->maxInputChannels);
        printf(", Max outputs = %d\n", dev_info->maxOutputChannels);

        if (extra_info)
        {
            printf("Default low input latency   = %8.4f\n", dev_info->defaultLowInputLatency);
            printf("Default low output latency  = %8.4f\n", dev_info->defaultLowOutputLatency);
            printf("Default high input latency  = %8.4f\n", dev_info->defaultHighInputLatency);
            printf("Default high output latency = %8.4f\n", dev_info->defaultHighOutputLatency);
        }

#ifdef WIN32
#if PA_USE_ASIO
        // todo
#endif
#endif
        printf("Default sample rate         = %8.2f\n", dev_info->defaultSampleRate);

        if (extra_info)
        {
            in_params.device = i;
            in_params.channelCount = dev_info->maxInputChannels;
            in_params.sampleFormat = paInt16;
            in_params.suggestedLatency = 0;
            in_params.hostApiSpecificStreamInfo = NULL;

            out_params.device = i;
            out_params.channelCount = dev_info->maxOutputChannels;
            out_params.sampleFormat = paInt16;
            out_params.suggestedLatency = 0;
            out_params.hostApiSpecificStreamInfo = NULL;

            if (in_params.channelCount > 0)
            {
                PrintSupportedStandardSampleRates(&in_params, NULL);
            }

            if (out_params.channelCount > 0)
            {
                PrintSupportedStandardSampleRates(NULL, &out_params);
            }

            if (in_params.channelCount > 0 && out_params.channelCount > 0)
            {
                PrintSupportedStandardSampleRates(&in_params, &out_params);
            }
        }
    }
    printf("----------------------------------------------\n");
}

int start_stream(int in, int out)
{
    PaStreamParameters in_params;
    PaStreamParameters out_params;
    PaStream* stream = NULL;
    PaError err;
    const PaDeviceInfo* in_info;
    const PaDeviceInfo* out_info;
    char* sample_block = NULL;
    int n_bytes;
    int n_chans;

    in_params.device = in;
    in_info = Pa_GetDeviceInfo(in_params.device);

    out_params.device = out;
    out_info = Pa_GetDeviceInfo(out_params.device);

    n_chans = in_info->maxInputChannels < out_info->maxOutputChannels ? in_info->maxInputChannels : out_info->maxOutputChannels;

    in_params.channelCount = n_chans;
    in_params.sampleFormat = PA_SAMPLE_TYPE;
    in_params.suggestedLatency = in_info->defaultLowInputLatency;
    //in_params.suggestedLatency = in_info->defaultHighInputLatency;
    in_params.hostApiSpecificStreamInfo = NULL;

    out_params.channelCount = n_chans;
    out_params.sampleFormat = PA_SAMPLE_TYPE;
    out_params.suggestedLatency = out_info->defaultLowInputLatency;
    //out_params.suggestedLatency = out_info->defaultHighInputLatency;
    out_params.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        &in_params,
        &out_params,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        NULL,
        NULL
    );

    if (err != paNoError)
    {
        return err;
    }

    n_bytes = FRAMES_PER_BUFFER * n_chans * SAMPLE_SIZE;
    sample_block = malloc(n_bytes);
    if (!sample_block)
    {
        Pa_StopStream(stream);
        return 1;
    }

    memset(sample_block, SAMPLE_SILENCE, n_bytes);

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        free(sample_block);
        return err;
    }

    while (running)
    {
        err = Pa_WriteStream(stream, sample_block, FRAMES_PER_BUFFER);
        if (err)
        {
            break;
        }

        err = Pa_ReadStream(stream, sample_block, FRAMES_PER_BUFFER);
        if (err)
        {
            break;
        }
    }
    if (err)
    {
        if (stream)
        {
            Pa_AbortStream(stream);
            Pa_CloseStream(stream);
        }
        free(sample_block);
    }

    return 0;
}

int main(int argc, char** argv)
{
    bool start = true;
    int n_devices;
    int input_device;
    int output_device;

    typedef void (*SignalHandlerPointer)(int);

    SignalHandlerPointer prevHandler;
    prevHandler = signal(SIGINT, signal_handler);

    init_audio(&n_devices);
    input_device = Pa_GetDefaultInputDevice();
    output_device = Pa_GetDefaultOutputDevice();

    for (int i = 1; i < argc; ++i)
    {
        if (strncmp(argv[i], "-l", 2) == 0)
        {
            start = false;

            bool extra_info = false;
            if (strncmp(argv[i], "-la", 3) == 0)
            {
                extra_info = true;
            }
            list_devices(n_devices, extra_info);
        }
        else if (strncmp(argv[i], "-i", 2) == 0)
        {
            input_device = atoi(argv[i] + 3);
        }
        else if (strncmp(argv[i], "-o", 2) == 0)
        {
            output_device = atoi(argv[i] + 3);
        }
        else
        {
            printf("ignoring unknown flag: %s\n", argv[i]);
        }
    }

    if (start)
    {
        request_high_priority();
        start_stream(input_device, output_device);
    }

    printf("shutting down\n");
    shutdown_audio();
    return 0;
}

