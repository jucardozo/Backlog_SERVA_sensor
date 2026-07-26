/*
 * circ_buf.h
 *
 *  Created on: 17 ene. 2024
 *      Author: joaco
 */

#ifndef JOACO_CIRCULAR_BUFFER_H_
#define JOACO_CIRCULAR_BUFFER_H_

#define BUFSIZE 400

typedef struct {
  unsigned char buffer[BUFSIZE];
  unsigned int input_index;
  unsigned int output_index;
  unsigned int distance;
} circular_buffer_t;

char pop_in_data(circular_buffer_t* buffer, unsigned char data);

unsigned char pop_out_data(circular_buffer_t* buffer);

void init_circular_buffer(circular_buffer_t* buffer);


#endif /* JOACO_CIRCULAR_BUFFER_H_ */
