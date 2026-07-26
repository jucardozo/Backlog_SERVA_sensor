#ifndef SENSOR_COMMUNICATION_H_
#define SENSOR_COMMUNICATION_H_

#include "i2c_driver.h"


#define ACC_SLAVE_ADDR   0x18
#define GYRO_SLAVE_ADDR  0x68
#define MAG_SLAVE_ADDR  0x1E

int8_t wakeup_acc(void);
int8_t get_acc_reading(uint16_t *accx,uint16_t *accy,uint16_t *accz);
int8_t turnoff_acc(void);
int8_t turnoff_gyro(void);

int8_t wakeup_magH(void);
int8_t wakeup_magL(void);
int8_t get_mag_reading(uint16_t *magx,uint16_t *magy,uint16_t *magz);
int8_t turnoff_mag(void);

#endif /* SENSOR_COMMUNICATION_H_ */
