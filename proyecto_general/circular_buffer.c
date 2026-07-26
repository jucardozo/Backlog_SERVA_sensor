/*
 * circular_buffer.c
 *
 *  Created on: 17 ene. 2024
 *      Author: joaco
 */

#include "circular_buffer.h"


char pop_in_data(circular_buffer_t* buffer, unsigned char data){

    if(buffer->distance > 0){
        buffer->buffer[buffer->input_index] = data;
        buffer->input_index++;
        if(buffer->input_index == BUFSIZE) buffer->input_index = 0;
        buffer->distance--;
        return 1;
    }
    else{
        return 0;
    }
}

unsigned char pop_out_data(circular_buffer_t* buffer){

    if(buffer->distance < BUFSIZE){
        buffer->distance++;
        unsigned char ret_aux = buffer->buffer[buffer->output_index];
        buffer->output_index++;
        if(buffer->output_index == BUFSIZE) buffer->output_index = 0;
        return ret_aux;
    }
    else{
        return 'e';
    }
}

void init_circular_buffer(circular_buffer_t* buffer){
    buffer->output_index = 0;
    buffer->input_index = 0;
    buffer->distance = BUFSIZE;
    int i = 0;
    for(i;i< BUFSIZE; i++){
        buffer->buffer[i] = 0;
    }
    return;
}


