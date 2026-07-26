/*
 * sensor_communication_New.h
 *
 *  Created on: 13 may. 2026
 *      Author: Salta
 */

#ifndef SENSOR_COMMUNICATION_NEW_H_
#define SENSOR_COMMUNICATION_NEW_H_

/*Nombre Provicional*/
/*Cambio de Hardware
*Magnetometro IIS2MDCTR 
*Accelerometo BMI323
*Cambia la direccion del registro. 0x68 si SDO pin is pulled to ’GND’.
*No se diferencia entre giroscopo y acelerometro. Comparten la misma direccion.*/

#include "i2c_driver.h"

//Acelerometro
#define ACC_SLAVE_ADDR   0x68
#define GYRO_SLAVE_ADDR  0x68

//Magnetometro
#define MAG_SLAVE_ADDR  0x1E          //0001 1110 Default Slave Address

int8_t wakeup_acc(void);
int8_t get_acc_reading(uint16_t *accx,uint16_t *accy,uint16_t *accz);
int8_t turnoff_acc(void);
int8_t turnoff_gyro(void);

int8_t wakeup_magH(void);
int8_t wakeup_magL(void);
int8_t get_mag_reading(uint16_t *magx,uint16_t *magy,uint16_t *magz);
int8_t turnoff_mag(void);

#endif /* SENSOR_COMMUNICATION_NEW_H_ */

/*Nombre Provicional*/
/*Cambio de Hardware
*Magnetometro IIS2MDCTR 
* Misma  familia de Magnetometro - Se mantiene la misma direccion de slave

*Accelerometo BMI323*/
