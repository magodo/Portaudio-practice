/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Thu 15 Dec 2016 01:03:41 PM CST
 File Name: swatooth.c
 Description: 
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

/***************** Type **********************/

typedef struct
{
    float left_phase;
    float right_phase;
}
paTestData;

/***************** Global Variable **********************/

#define NUM_SECONDS 5

static paTestData data;

/***************** Function **********************/

void exit_error(PaError err, const char *msg)
{
    printf("%s: %s\n", msg, Pa_GetErrorText(err));
    exit(-1);
}

/* This routing will be called by the PortAudio engine when audio is needed.
 * It may be called at interrupt level(although in some case there is a 
 * dedicated thread instead) on some machines. So don't do anything that could
 * mess up the system:
 *   * Use "thread safe" functions
 *   * Only call "reentrant" functions, not including:
 *      - functions using static data
 *      - `free` or `malloc`
 *      - standard I/O APIs
 *   * Don't use locks which are used also by other threads
 */
static int patestCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    // cast "void*" parameters
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;

    unsigned int i;

    for (i = 0; i < framesPerBuffer; i++)
    {
        *out++ = data->left_phase;
        *out++ = data->right_phase;

        data->left_phase += 0.01f;
        data->right_phase += 0.03f; // right channel has higher pitch
        if (data->left_phase >= 1.0f) data->left_phase -= 2.0f;
        if (data->right_phase >= 1.0f) data->right_phase -= 2.0f;
    }
    return 0; // stream will be stopped if callback return 1
}

int main()
{
    PaError err;
    PaStream *stream;

    // initialize PortAudio library
    err = Pa_Initialize();
    if (err != paNoError) exit_error(err, "Pa_Initialize failed");
    
    // open a stream using defaults
    err = Pa_OpenDefaultStream(&stream,
                               0,               /* no input channels */
                               2,               /* stereo output */
                               paFloat32,       /* 32 bit floating point output */
                               16000,           /* sample rate */
                               256,             /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps 
                                                   may want to use paFramesPerBufferUnspecified,
                                                   which tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
                               patestCallback,  /* callback function defined by user */
                               &data );         /* pointer to custom data which will be passed to callback */
    if (err != paNoError) exit_error(err, "Pa_OpenDefaultStream failed");

    // start stream
    err = Pa_StartStream(stream);
    if (err != paNoError) exit_error(err, "Pa_StartStream failed");

    // sleep some time
    Pa_Sleep(NUM_SECONDS * 1000);

    /* stop/abort stream in controlling thread
     * alternatively, you can stop/abort stream by returning paComplete or paAbort from callback,
     * however, this requests another call to `Pa_StopStream()` so that you can start stream again.
     */
    err = Pa_StopStream(stream);
    if (err != paNoError) exit_error(err, "Pa_StopStream failed");

    // close stream
    err = Pa_CloseStream(stream);
    if (err != paNoError) exit_error(err, "Pa_CloseStream failed");

    // terminate PA library
    err = Pa_Terminate();
    if (err != paNoError) exit_error(err, "Pa_Terminate failed");
}

