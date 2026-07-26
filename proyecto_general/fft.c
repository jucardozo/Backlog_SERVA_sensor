/*
 * fft.c
 *
 *  Created on: 24 mar. 2024
 *      Author: joaco
 */


#include "fft.h"


msp_status do_fft(_q15 * input){

    /* Multiply input signal by generated Hamming window */
    msp_mpy_q15_params mpyParams;
    mpyParams.length = SAMPLES;
    //status = msp_mpy_q15(&mpyParams, input, window, input);
    //msp_checkStatus(status);

    /* Initialize the fft parameter structure. */
    msp_cmplx_fft_q15_params fftParams;
    fftParams.length = SAMPLES;
    fftParams.bitReverse = true;
    fftParams.twiddleTable = MAP_msp_cmplx_twiddle_table_2048_q15;

    /* Perform real FFT with fixed scaling */

    msp_status status;

    status = msp_cmplx_fft_fixed_q15(&fftParams, input);
    msp_checkStatus(status);

    return status;

}



