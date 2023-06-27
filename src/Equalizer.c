#include "Equalizer.h"
#include "FirFilter.h"

#define PI 3.1415926

// ref:https://github.com/zuguorui/EQ-iir/blob/master/Equalizer.cpp
Equalizer *createEqualizer(int numChannel, int sampleRate)
{
    Equalizer *equalizer = (Equalizer *)malloc(sizeof(Equalizer));
    if (equalizer == NULL)
    {
        return NULL;
    }
    generateFilter(equalizer);
    equalizer->m_filt_t=0;// LPF
    
    equalizer->numChannel = numChannel;
    equalizer->sampleRate = sampleRate;
    equalizer->bassGain = 1.0;
    equalizer->midGain = 1.0;
    equalizer->trebleGain = 1.0;
    equalizer->gain = pow(10, 6.0 / 20);
    equalizer->inputSamples = 0;
    equalizer->processBandTimes = 0;
    equalizer->channelInputs = (double **)malloc(numChannel * sizeof(double *));
    if (equalizer->channelInputs == NULL)
    {
        free(equalizer);
        return NULL;
    }

    for (int i = 0; i < numChannel; i++)
    {
        equalizer->channelInputs[i] = (double *)malloc(SAMPLES_PER_BUFFER * sizeof(double));
        if (equalizer->channelInputs[i] == NULL)
        {
            for (int j = i - 1; j >= 0; j--)
            {
                free(equalizer->channelInputs[j]);
            }
            free(equalizer->channelInputs);
            free(equalizer);
            return NULL;
        }
    }


    return equalizer;
}

void destroyEqualizer(Equalizer *equalizer)
{
    if (equalizer != NULL)
    {
        if (equalizer->channelInputs != NULL)
        {
            for (int i = 0; i < equalizer->numChannel; i++)
            {
                free(equalizer->channelInputs[i]);
            }
            free(equalizer->channelInputs);
        }
        if(equalizer->CH1BPFFir!=NULL)
        {
            free(equalizer->CH1BPFFir);
        }
        if(equalizer->CH1LPFFir!=NULL)
        {
            free(equalizer->CH1LPFFir);
        }
        if(equalizer->CH1HPFFir!=NULL)
        {
            free(equalizer->CH1HPFFir);
        }
        if(equalizer->CH2BPFFir!=NULL)
        {
            free(equalizer->CH2BPFFir);
        }
        if(equalizer->CH2LPFFir!=NULL)
        {
            free(equalizer->CH2LPFFir);
        }
        if(equalizer->CH2HPFFir!=NULL)
        {
            free(equalizer->CH2HPFFir);
        }
        free(equalizer);
    }
}

void generateFilter(Equalizer *equalizer)
{
    equalizer->CH1LPFFir=ConstructFirFilterV1(LPF, 51, 44.1, 0.25);
    equalizer->CH1HPFFir=ConstructFirFilterV1(HPF, 51, 44.1, 3.0);
    equalizer->CH1BPFFir=ConstructFirFilterV2(BPF, 51, 44.1, 0.25, 3.0);

    equalizer->CH2LPFFir=ConstructFirFilterV1(LPF, 51, 44.1, 0.25);
    equalizer->CH2HPFFir=ConstructFirFilterV1(HPF, 51, 44.1, 3.0);
    equalizer->CH2BPFFir=ConstructFirFilterV2(BPF, 51, 44.1, 0.25, 3.0);
}



void processChannel(Equalizer *equalizer, double *channel, int numSample, int isLeftChannel)
{
    if(isLeftChannel)
    {
        if(equalizer->m_filt_t==LPF){
            processFir(equalizer->CH1LPFFir,channel, equalizer->lp_out, numSample);
        }
        else if(equalizer->m_filt_t==BPF){
            processFir(equalizer->CH1BPFFir,channel, equalizer->bp_out, numSample);
        }
        else if(equalizer->m_filt_t==HPF){
            processFir(equalizer->CH1HPFFir,channel, equalizer->hp_out, numSample);
        }
    }
    else{
        if(equalizer->m_filt_t==LPF){
            processFir(equalizer->CH2LPFFir,channel, equalizer->lp_out, numSample);
        }
        else if(equalizer->m_filt_t==BPF){
            processFir(equalizer->CH2BPFFir,channel, equalizer->bp_out, numSample);
        }
        else if(equalizer->m_filt_t==HPF){
            processFir(equalizer->CH2HPFFir,channel, equalizer->hp_out, numSample);
        }
    }

    for (int i = 0; i < numSample; i++)
    {
        double d =channel[i];
        if(equalizer->m_filt_t==LPF){
            d=d+equalizer->lp_out[i] * equalizer->bassGain;
        }
        else if(equalizer->m_filt_t==BPF){
            d=d+equalizer->bp_out[i] * equalizer->midGain;
        }
        else if(equalizer->m_filt_t==HPF){
            d=equalizer->hp_out[i] * equalizer->trebleGain;
        }

        // double d = equalizer->lp_out[i] * equalizer->bassGain +
        //            equalizer->bp_out[i] * equalizer->midGain +
        //            equalizer->hp_out[i] * equalizer->trebleGain;
        channel[i] = d;
    }
}

void separateChannel(Equalizer *equalizer, int16_t *input, int numSample)
{
    for (int i = 0; i < equalizer->numChannel; i++)
    {
        double *channel = equalizer->channelInputs[i];
        for (int j = 0; j < numSample; j++)
        {
            channel[j] = input[equalizer->numChannel * j + i] * equalizer->gain / INT16_MAX;
        }
    }
}

void mixChannel(Equalizer *equalizer, int16_t *output, int numSample)
{
    for (int i = 0; i < equalizer->numChannel; i++)
    {
        double *channel = equalizer->channelInputs[i];
        for (int j = 0; j < numSample; j++)
        {
            double data = channel[j];
            if (data > 1)
            {
                data = 1;
            }
            else if (data < -1)
            {
                data = -1;
            }
            output[equalizer->numChannel * j + i] = (int16_t)(data * INT16_MAX);
        }
    }
}

void setBassGain(Equalizer *equalizer, double gain)
{
    equalizer->midGain = 1;
    equalizer->trebleGain = 1;
    equalizer->bassGain = pow(10, gain / 20);
    equalizer->gain = pow(10, 6.0 / 20);
}

void setMidGain(Equalizer *equalizer, double gain)
{
    equalizer->midGain = pow(10, gain / 20);
    equalizer->bassGain = 1;
    equalizer->trebleGain = 1;
    equalizer->gain = pow(10, 6.0 / 20);
}

void setTrebleGain(Equalizer *equalizer, double gain)
{
    equalizer->trebleGain = pow(10, gain / 20);
    equalizer->midGain = 1;
    equalizer->bassGain = 1;
    equalizer->gain = pow(10, 6.0 / 20);
}

void setGain(Equalizer *equalizer, double gain)
{
    equalizer->midGain = 1;
    equalizer->trebleGain = 1;
    equalizer->bassGain = 1;
    equalizer->gain = pow(10, gain / 20);
}

void eqProcess(Equalizer *equalizer, int16_t *data, int numSample)
{
    separateChannel(equalizer, data, numSample);

    // Process left channel
    double *channel1 = equalizer->channelInputs[0];
    processChannel(equalizer, channel1, numSample, 0);

    // Process right channel
    double *channel2 = equalizer->channelInputs[1];
    processChannel(equalizer, channel2, numSample, 1);
    mixChannel(equalizer,data, numSample);

}
