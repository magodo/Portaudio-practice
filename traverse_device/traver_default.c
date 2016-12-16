/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Thu 15 Dec 2016 02:31:58 PM CST
 File Name: travers.c
 Description: 
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

void exit_error(PaError err, const char *msg)
{
    printf("%s: %s\n", msg, Pa_GetErrorText(err));
    exit(-1);
}

int main()
{
    int num_device;
    PaError err;

    // Init
    Pa_Initialize();

    // get device count
    num_device = Pa_GetDeviceCount();
    if (num_device < 0) exit_error(num_device, "Pa_GetDeviceCount failed");

    // traverse each devices for default configurations
    const PaDeviceInfo *deviceInfo;

    int i;
    for (i = 0; i < num_device; i++)
    {
        deviceInfo = Pa_GetDeviceInfo(i);

        printf("struct version              : %d\n", deviceInfo->structVersion);
        printf("name                        : %s\n", deviceInfo->name);
        printf("hostApi                     : %d\n", deviceInfo->hostApi);
        printf("max input channels          : %d\n", deviceInfo->maxInputChannels);
        printf("max output channels         : %d\n", deviceInfo->maxOutputChannels);
        printf("default low input latency   : %f\n", deviceInfo->defaultLowInputLatency);
        printf("default low output latency  : %f\n", deviceInfo->defaultLowOutputLatency);
        printf("default high input latency  : %f\n", deviceInfo->defaultHighInputLatency);
        printf("default high output latency : %f\n", deviceInfo->defaultHighOutputLatency);
        printf("default sample rate         : %f\n", deviceInfo->defaultSampleRate);
    }
    
    // terminate
    Pa_Terminate();
}

