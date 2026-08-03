/*
 * auxfuncs.h
 *
 *  Created on: 28 mar. 2024
 *      Author: joaco
 */

#ifndef AUXFUNCS_H_
#define AUXFUNCS_H_

#include <stdint.h>

void append_and_shift(int16_t * array, int array_len, int16_t newitem);
int16_t get_mean(int16_t * array, int16_t arraylen);

#endif /* AUXFUNCS_H_ */
