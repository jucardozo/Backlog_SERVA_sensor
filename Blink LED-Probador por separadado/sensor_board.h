/*
 * sensor_board.h
 *
 *  Created on: 20 abr. 2024
 *      Author: joaco
 */

#ifndef SENSOR_BOARD_H_
#define SENSOR_BOARD_H_

#include <stdint.h>

void read_mag(int16_t* magx,int16_t* magy,int16_t* magz);
void read_acc(int16_t* accx,int16_t* accy,int16_t* accz);
void read_gyro(int16_t* gyrx, int16_t* gyry, int16_t* gyrz);
void init_magacc_driver();
void reinit_magacc_driver();



#endif /* SENSOR_BOARD_H_ */
