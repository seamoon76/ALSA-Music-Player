// ref:https://github.com/mihaibujanca/soundGR/blob/master/FIR-filter-class/filt.h
#ifndef _FILTER_H
#define _FILTER_H

#define MAX_NUM_FILTER_TAPS 1000

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>


// enum filterType {LPF, HPF, BPF};
#define LPF 0
#define HPF 1
#define BPF 2

struct FirFilter
{
	/* data */

	int m_filt_t;//LPF, HPF, BPF对应0,1,2
	int m_num_taps;
	int m_error_flag;
	double m_Fs;
	double m_Fx;
	double m_lambda;
	double *m_taps;
	double *m_sr;
	double m_Fu, m_phi;
		
};

void designLPF(struct FirFilter* fir);
void designHPF(struct FirFilter* fir);

// Only needed for the bandpass filter case

void designBPF(struct FirFilter* fir);

struct FirFilter* ConstructFirFilterV1(int filt_t, int num_taps, double Fs, double Fx);
struct FirFilter* ConstructFirFilterV2(int filt_t, int num_taps, double Fs, double Fl, double Fu);
void DestroyFirFilter( struct FirFilter* fir);
void initFir(struct FirFilter* fir);
double do_sample(struct FirFilter* fir,double data_sample);
int get_error_flag(struct FirFilter* fir);
void get_taps(struct FirFilter* fir, double *taps );
void processFir(struct FirFilter* fir,double *input, double *output, int len);
#endif
