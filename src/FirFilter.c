// ref:https://github.com/mihaibujanca/soundGR/blob/master/FIR-filter-class/filt.cpp
#include "FirFilter.h"
#include <math.h>
#define M_PI       3.14159265358979323846   // pi
// #include <chrono>
void ECODE( struct FirFilter* fir,int x) {fir->m_error_flag = x;return;}

int get_error_flag(struct FirFilter* fir){return fir->m_error_flag;};
// Handles LPF and HPF case
struct FirFilter* ConstructFirFilterV1(int filt_t, int num_taps, double Fs, double Fx)
{
	struct FirFilter *fir = (struct FirFilter *)malloc(sizeof(struct FirFilter));
    if (fir == NULL)
    {
        return NULL;
    }
	fir->m_error_flag = 0;
	fir->m_filt_t = filt_t;
	fir->m_num_taps = num_taps;
	fir->m_Fs = Fs;
	fir->m_Fx = Fx;
	fir->m_lambda = M_PI * Fx / (Fs/2);

	if( Fs <= 0 ) ECODE(fir, -1);
	if( Fx <= 0 || Fx >= Fs/2 ) ECODE(fir, -2);
	if( fir->m_num_taps <= 0 || fir->m_num_taps > MAX_NUM_FILTER_TAPS ) ECODE(fir, -3);

	fir->m_taps = fir->m_sr = NULL;
	fir->m_taps = (double*)malloc( fir->m_num_taps * sizeof(double) );
	fir->m_sr = (double*)malloc( fir->m_num_taps * sizeof(double) );
	// printf("fir->m_sr[0]=%f\n",fir->m_sr[0]);
	if( fir->m_taps == NULL || fir->m_sr == NULL ) ECODE(fir, -4);
	
	initFir(fir);

	if( fir->m_filt_t == LPF ) designLPF(fir);
	else if( fir->m_filt_t == HPF ) designHPF(fir);
	else ECODE(fir, -5);

	return fir;
}

// Handles BPF case
struct FirFilter* ConstructFirFilterV2(int filt_t, int num_taps, double Fs, double Fl,
               double Fu)
{
	struct FirFilter *fir = (struct FirFilter *)malloc(sizeof(struct FirFilter));
    if (fir == NULL)
    {
        return NULL;
    }
	fir->m_error_flag = 0;
	fir->m_filt_t = filt_t;
	fir->m_num_taps = num_taps;
	fir->m_Fs = Fs;
	fir->m_Fx = Fl;
	fir->m_Fu = Fu;
	fir->m_lambda = M_PI * Fl / (Fs/2);
	fir->m_phi = M_PI * Fu / (Fs/2);

	if( Fs <= 0 ) ECODE(fir, -10);
	if( Fl >= Fu ) ECODE(fir, -11);
	if( Fl <= 0 || Fl >= Fs/2 ) ECODE(fir, -12);
	if( Fu <= 0 || Fu >= Fs/2 ) ECODE(fir, -13);
	if( fir->m_num_taps <= 0 || fir->m_num_taps > MAX_NUM_FILTER_TAPS ) ECODE(fir, -14);

	fir->m_taps = fir->m_sr = NULL;
	fir->m_taps = (double*)malloc( fir->m_num_taps * sizeof(double) );
	fir->m_sr = (double*)malloc( fir->m_num_taps * sizeof(double) );
	if( fir->m_taps == NULL || fir->m_sr == NULL ) ECODE(fir, -15);
	
	initFir(fir);

	if( fir->m_filt_t == BPF ) designBPF(fir);
	else ECODE(fir, -16);

	return fir;
}

void DestroyFirFilter(struct  FirFilter* fir)
{
	if( fir->m_taps != NULL ) free( fir->m_taps );
	if( fir->m_sr != NULL ) free( fir->m_sr );
}

void designLPF(struct FirFilter* fir)
{
	int n;
	double mm;

	for(n = 0; n < fir->m_num_taps; n++){
		mm = n - (fir->m_num_taps - 1.0) / 2.0;
		if( mm == 0.0 ) fir->m_taps[n] = fir->m_lambda / M_PI;
		else fir->m_taps[n] = sin( mm * fir->m_lambda ) / (mm * M_PI);
	}

	return;
}

void 
designHPF(struct FirFilter* fir)
{
	int n;
	double mm;

	for(n = 0; n < fir->m_num_taps; n++){
		mm = n - (fir->m_num_taps - 1.0) / 2.0;
		if( mm == 0.0 ) fir->m_taps[n] = 1.0 - fir->m_lambda / M_PI;
		else fir->m_taps[n] = -sin( mm * fir->m_lambda ) / (mm * M_PI);
	}

	return;
}

void 
designBPF(struct FirFilter* fir)
{
	int n;
	double mm;

	for(n = 0; n < fir->m_num_taps; n++){
		mm = n - (fir->m_num_taps - 1.0) / 2.0;
		if( mm == 0.0 ) fir->m_taps[n] = (fir->m_phi - fir->m_lambda) / M_PI;
		else fir->m_taps[n] = (   sin( mm * fir->m_phi ) -
		                     sin( mm * fir->m_lambda )   ) / (mm * M_PI);
	}

	return;
}

void 
get_taps( struct FirFilter* fir,double *taps )
{
	int i;

	if( fir->m_error_flag != 0 ) return;

	for(i = 0; i < fir->m_num_taps; i++) taps[i] = fir->m_taps[i];

  return;		
}


// Output the magnitude of the frequency response in dB
#define NP 1000

void initFir(struct FirFilter* fir)
{
	int i;

	if( fir->m_error_flag != 0 ) return;

	for(i = 0; i < fir->m_num_taps; i++) fir->m_sr[i] = 0;

	return;
}

double do_sample(struct FirFilter* fir,double data_sample)
{
	int i;
	double result;

	if( fir->m_error_flag != 0 ) return(0);
	// printf("fir address value:%d\n",fir);

	// printf("fir->m_sr[0] address value:%f\n",fir->m_sr[0]);
	for(i = fir->m_num_taps - 1; i >= 1; i--){
		fir->m_sr[i] = fir->m_sr[i-1];
	}	
	fir->m_sr[0] = data_sample;
	// printf("160\n");
	result = 0;
	for(i = 0; i < fir->m_num_taps; i++) result += fir->m_sr[i] * fir->m_taps[i];

	return result;
}

void processFir(struct FirFilter* fir,double *input, double *output, int len)
{
    for(int i = 0; i < len; i++)
    {

        double d = do_sample(fir,input[i]);
        output[i] = d;

    }

}
