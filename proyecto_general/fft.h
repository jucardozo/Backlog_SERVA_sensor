/*
 * fft.h
 *
 *  Created on: 24 mar. 2024
 *      Author: joaco
 */

#ifndef FFT_H_
#define FFT_H_


#include "DSPLib.h"

#define SAMPLES 512

msp_status do_fft(_q15 * input);

#endif /* FFT_H_ */
