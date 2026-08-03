/*
 * auxfuncs.c
 *
 *  Created on: 28 mar. 2024
 *      Author: joaco
 */

#include "auxfuncs.h"


int16_t get_mean(int16_t * array, int16_t arraylen){
    int16_t x = 0;
    int64_t sum = 0;
    for(x; x<arraylen; x++){
        sum += (int64_t)array[x];
    }

    int16_t retval;



    retval = (int16_t) (sum/((int64_t)arraylen));
    return retval;
}


void append_and_shift(int16_t * array, int array_len, int16_t newitem) {
    int p = 1;

    for(p;p<array_len;p++){
        array[p-1] = array[p];
    }

    array[array_len-1] = newitem;

    return;
}
