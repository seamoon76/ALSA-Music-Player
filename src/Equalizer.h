//ref : https://github.com/zuguorui/EQ-iir/blob/master/Equalizer.h

#ifndef _EQUALIZER_H_
#define _EQUALIZER_H_

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "FirFilter.h"

// Maximum number of samples processed per buffer
#define SAMPLES_PER_BUFFER  6144

/**
 * Equalizer structure represents the main equalizer.
 */
typedef struct
{
    int numChannel;
    int sampleRate;
    double bassGain;
    double midGain;
    double trebleGain;
    double gain;
    int inputSamples;
    int processBandTimes;
    double **channelInputs;
    double lp_out[SAMPLES_PER_BUFFER];
    double bp_out[SAMPLES_PER_BUFFER];
    double hp_out[SAMPLES_PER_BUFFER];
    struct FirFilter* CH1LPFFir;
    struct FirFilter* CH1HPFFir;
    struct FirFilter* CH1BPFFir;
    struct FirFilter* CH2LPFFir;
    struct FirFilter* CH2HPFFir;
    struct FirFilter* CH2BPFFir;
    int m_filt_t;
} Equalizer;

// Function declarations
Equalizer *createEqualizer(int numChannel, int sampleRate);
void destroyEqualizer(Equalizer *equalizer);
void generateFilter(Equalizer *equalizer);
void eqProcess(Equalizer *equalizer, int16_t *data, int numSample);
void setBassGain(Equalizer *equalizer, double gain);
void setMidGain(Equalizer *equalizer, double gain);
void setTrebleGain(Equalizer *equalizer, double gain);
void setGain(Equalizer *equalizer, double gain);

#endif

