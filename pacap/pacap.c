/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Thu 15 Dec 2016 09:20:30 PM CST
Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "portaudio.h"

/***********
 * Declare
 **********/

struct subcommand
{
    const char *name;
    int (*func)(int argc, char *argv[]);
};

struct stream_format
{
    const char *name;
    PaSampleFormat macro;
};

static int test_format(int argc, char *argv[]);
static int traverse(int argc, char *argv[]);

/*******************
 * Global variables
 *******************/

/* used to store name of this program */
static char *program_name = 0;

static const struct subcommand subcommands[] = {
    {"test_format",test_format},
    {"traverse", traverse}
};

static const struct stream_format pa_format[] = {
    {"f32", paFloat32},
    {"i32", paInt32},
    {"i24", paInt24},
    {"i16", paInt16},
    {"i8", paInt8},
    {"u8", paUInt8}
};

/*********************************
 * Utility
 *********************************/

void exit_error(PaError err, const char *msg)
{
    printf("%s: %s\n", msg, Pa_GetErrorText(err));
    exit(-1);
}

// format name -> pa macro
static PaSampleFormat format_name_to_macro(const char *name)
{
    if (name == NULL)
    {
        printf("Format name is NULL\n");
        exit(-1);
    }

    unsigned i;
    for (i = 0; i < sizeof(pa_format)/sizeof(pa_format[0]); ++i)
    {
        if (!strcmp(name, pa_format[i].name))
            return pa_format[i].macro;
    }

    printf("Unknown format name: %s\n", name);
    exit(-1);
}

// format pa macro -> format name
static const char *format_macro_to_name(PaSampleFormat macro)
{
    unsigned i;
    for (i = 0; i < sizeof(pa_format)/sizeof(pa_format[0]); ++i)
    {
        if (macro == pa_format[i].macro)
            return pa_format[i].name;
    }
    printf("Unknown format macro: %lu\n", macro);
    exit(-1);
}

/*******************************************************
 * Usage function for every subcommand and the program itself.
 *******************************************************/

static void usage(const char *subcommand)
{
    /* no subcommand is specified */
    if (subcommand == NULL)
    {
        printf("Usage: %s [COMMAND] [OPTION]\n", program_name);

        printf("\nSupported [COMMAND] includes:\n");
        unsigned i;
        unsigned n_of_subcommands = sizeof(subcommands)/sizeof(subcommands[0]);
        for (i = 0; i < n_of_subcommands; ++i)
            printf("* %s\n", subcommands[i].name);
    }
    else if (!strcmp(subcommand, "traverse"))
    {
        printf("Usage: %s %s [OPTION]\n\n",program_name, subcommand);
        printf("-h, --help                  help\n");
    }

    else if (!strcmp(subcommand, "test_format"))
    {
        printf("Usage: %s %s [OPTION] [DEVICE INDEX]\n\n",program_name, subcommand);
        printf("-h, --help                  help\n");
        printf("-c, --channel=#             channel count\n");
        printf("-f, --format=FORMAT         sample format\n");
        printf("-l, --latency=#             latency\n");
        printf("-n, --nointerleaved         store different channels' samples in different buffers\n");
        printf("-r, --rate                  sample rate (e.g. 48000, 44100,...)\n");
        printf("\n\nSupported format includes: f32, i32, i24, i16, i8, u8\n");
    }
    // no need to check "else" cases, this is guaranteed because it will only be called by corresponding function
}

/************************
 * Harness function
 * *********************/

static void do_traverse()
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

        printf("\n");
        printf("device index                : %d\n", i);
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

static int do_test_format(PaDeviceIndex device_idx, int input_channel, int output_channel, PaTime input_latency, 
                          PaTime output_latency, const char *format, int is_noninterleaved, double rate)
{
    // init lib
    Pa_Initialize();


    // construct PaSampleFormat
    PaSampleFormat sample_format = format_name_to_macro(format);
    if (is_noninterleaved)
        sample_format |= paNonInterleaved;

    // construct stream parameter
    PaStreamParameters expect_input_param;
    PaStreamParameters expect_output_param;

    expect_input_param.device = device_idx;
    expect_input_param.channelCount = input_channel;
    expect_input_param.sampleFormat = sample_format;
    expect_input_param.suggestedLatency = input_latency;
    expect_input_param.hostApiSpecificStreamInfo = NULL;

    expect_output_param.device = device_idx;
    expect_output_param.channelCount = output_channel;
    expect_output_param.sampleFormat = sample_format;
    expect_output_param.suggestedLatency = output_latency;
    expect_output_param.hostApiSpecificStreamInfo = NULL;

    // test device parameters
    PaError err;

    err = Pa_IsFormatSupported(&expect_input_param, NULL, rate);
    printf("Open this stream as input with following parameters:\n");
    printf("* channel       : %d\n", expect_input_param.channelCount);
    printf("* format        : %s\n", format);
    printf("* is_interleaved: %s\n", is_noninterleaved?"no":"yes");
    printf("* latency (sec) : %f\n", expect_input_param.suggestedLatency);
    printf("* rate (Hz)     : %f\n", rate);
    if (err == paFormatIsSupported)
        printf("\nSupported\n");
    else
        printf("\nNot supported: %s\n", Pa_GetErrorText(err));

    err = Pa_IsFormatSupported(NULL, &expect_output_param, rate);
    printf("Open this stream as output with following parameters:\n");
    printf("* channel       : %d\n", expect_output_param.channelCount);
    printf("* format        : %s\n", format);
    printf("* is_interleaved: %s\n", is_noninterleaved?"no":"yes");
    printf("* latency (sec) : %f\n", expect_output_param.suggestedLatency);
    printf("* rate (Hz)     : %f\n", rate);
    if (err == paFormatIsSupported)
        printf("\nSupported\n");
    else
        printf("\nNot supported: %s\n", Pa_GetErrorText(err));

    // terminate
    Pa_Terminate();

    return 0;
}

/************************
 * Sub-command functions
 * *********************/

static int traverse(int argc, char *argv[])
{

    optind = 1; // reset the index

    const char *optstring = ":h";
    const struct option longopts[] = {
        {"help", no_argument, NULL, 'h'},
        {0,0,0,0}
    };

    int val;
    while ((val = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
    {
        switch (val)
        {
            case 'h':
                usage(argv[0]);
                return 0;
            case '?':
                printf("unknown option: %c\n", optopt);
                return -1;
            case ':':
                printf("option requires an argument -- %c\n", optopt);
                return -1;
            default:
                printf("will never reach here\n");
                return -1;
        }
    }

    do_traverse();

    return 0;
}

static int test_format(int argc, char *argv[])
{
    
    optind = 1; // reset the index
    int val;

    const char *optstring = ":hc:f:l:nr:";
    const struct option longopts[] = {
        {"help", no_argument, NULL, 'h'},
        {"channel", required_argument, NULL, 'c'},
        {"format", required_argument, NULL, 'f'},
        {"latency", required_argument, NULL, 'l'},
        {"noninterleaved", no_argument, NULL, 'n'},
        {"rate", required_argument, NULL, 'r'},
        {0,0,0,0}
    };

    /* Step 1. parse option for the first time to just get the device index */

    while ((val = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) 
    {
        switch (val)
        {
            case 'h':
                usage(argv[0]);
                return 0;
            case '?':
                printf("unknown option: %c\n", optopt);
                return -1;
            case ':':
                printf("option requires an argument -- %c\n", optopt);
                return -1;
            default:
                // not care other options
                break;
        }
    }
    // now optind points to the first non-option argv-element or ending '\0' of argv
    switch (argc-optind)
    {
        case 0:
            // no non-option argv-element specified
            printf("Please choose specify device index to check!\n");
            return -1;
        case 1:
            // good case
            break;
        default:
            printf("Warning: multiple device indexes are specified, only the first one is taken\n");
            break;
    }
    int arg_device_idx = strtol(argv[optind], NULL, 0);


    /* Step 2. set expected device parameter from device info */

    PaError err;

    // init lib
    Pa_Initialize();

    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(arg_device_idx);

    int arg_input_channel = deviceInfo->maxInputChannels;
    int arg_output_channel = deviceInfo->maxOutputChannels;
    double arg_input_latency = deviceInfo->defaultLowInputLatency;
    double arg_output_latency = deviceInfo->defaultLowOutputLatency;
    double arg_rate = deviceInfo->defaultSampleRate;

    // other parameters are not available from device info, set them to some sane defaults
    char *arg_format = "f32";
    int arg_is_noninterleaved = 0; // by default PA pass data as a single buffer with all channels interleaved 

    // uninit lib
    Pa_Terminate();


    /* Step 3. Re-process options to set expected device parameter from command line on top of default value */

    optind = 1;
    while ((val = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
    {
        switch (val)
        {
            case 'c':
                arg_input_channel = arg_output_channel = strtol(optarg, NULL, 0);
                break;
            case 'f':
                arg_format = strdup(optarg);
                break;
            case 'l':
                arg_input_latency = arg_output_latency = strtod(optarg, NULL);
                break;
            case 'n':
                arg_is_noninterleaved = 1;
                break;
            case 'r':
                arg_rate = strtod(optarg, NULL);
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            case '?':
                printf("unknown option: %c\n", optopt);
                return -1;
            case ':':
                printf("option requires an argument -- %c\n", optopt);
                return -1;
            default:
                printf("will never reach here\n");
                return -1;
        }
    }


    return do_test_format(arg_device_idx, arg_input_channel, arg_output_channel, arg_input_latency, arg_output_latency,
                          arg_format, arg_is_noninterleaved, arg_rate);
}

/*************
 * MAIN
 *************/

int main(int argc, char *argv[])
{
    int ret;

    /* array of function names of */
    optind = 1;
    opterr = 0;

    /* store program name in global variable */
    program_name = strdup(argv[0]);

    /* check if there is a sub-command(it is guaranteed to be the argv[1] element)*/
    if (argv[1] != '\0' && *argv[1] != '-')
    {
        /* A subcommand is specified */

        unsigned index;
        unsigned n_of_subcommands = sizeof(subcommands)/sizeof(subcommands[0]);
        for (index = 0; index < n_of_subcommands; ++index)
        {
            if (!strcmp(subcommands[index].name, argv[1]))
            {
                /* exclude the first argv-element */
                --argc;
                argv = &argv[1];

                /* call subcommand function */
                ret = subcommands[index].func(argc, argv);
                break;
            }
        }
        if (index == n_of_subcommands)
        {
            printf("Unknown subcommand: %s\n", argv[1]);
            ret = -1;
        }
    }
    else
    {
        /* No subcommand specified, run program directly*/
        const char *optstring = ":h";
        const struct option longopts[] = {
            {"help", required_argument, NULL, 'h'},
            {0,0,0,0}
        };
        int val;

        while ((val = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
        {
            switch (val)
            {
                case 'h':
                    usage(NULL);
                    return 0;
                case '?':
                    printf("unknown option: %c\n", optopt);
                    return -1;
                case ':':
                    printf("option requires an argument -- %c\n", optopt);
                    return -1;
                default:
                    printf("will never reach here\n");
                    return -1;
            }
        }
    }

    /* free allocated memeory before leave */
    free(program_name);

    return 0;
}
