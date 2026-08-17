/*
 * sensor_communication.c  
 *  Created on: 13 may. 2026
 *      Author: Salta
 */



/*Nombre Provicional*/
/*Cambio de Hardware
*Magnetometro IIS2MDCTR 
* Direccion de los registro bien. 
*Accelerometo BMI323
*El nuevo chip es de 16bitss. little endian
*Distintos registros*/

#include "sensor_communication.h"

//Acelerometro
#define BMI323_CMD_REG 0x7E
#define BMI323_CHIP_ID_REG 0x00

#define ACC_CONF_REG  0x20
//#define ACC_PWR_CONF 0x7C
#define ACC_DATAX_REG 0x03
//#define ACC_XLSB_REG  0x12

//Giroscopo
#define GYRO_CONF_REG 0x21
#define GYRO_DATAX_REG 0x06
//#define GYRO_LPM1 0x11

//Magnetometro 
#define CFG_REG_A 0x60 //mag config    
#define CFG_REG_B 0x61 //mag config
#define MAG_XLSB_REG  0x68

extern uint8_t buffer[8] = {0};
extern uint8_t cmd_data[2] = {0};

int8_t wakeup_acc(void){
    //little endian.
    cmd_data[0] =0x08;  //   acc_bw[0]=BW = acc_odr/2; acc_range[000]=+/-2g, 16.38 LSB/mg ; acc_odr[1000] =ODR = 100Hz
    cmd_data[1] =0x70;  //   0+acc_mode[111]=?; 0+acc_avg_num[000]=No averaging; pass sample without filtering
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, ACC_CONF_REG, cmd_data, 2);
    _delay_cycles(50000); // wait 1ms for boot time on the sensor, just needed the first time


    /*
     * Se puede configurar el rango: Reg 0x41
     * 0x00(default +-3g)
     * 0x01(default +-6g)
     * 0x02(default +-12g)
     * 0x03(default +-24g)
     */
}

int8_t turnoff_acc(void){
       //little endian.
    cmd_data[0] =0x08;  //   acc_bw[0]=BW = acc_odr/2; acc_range[000]=+/-2g, 16.38 LSB/mg ; acc_odr[1000] =ODR = 100Hz
    cmd_data[1] =0x00;  //   0+acc_mode[000]=disables the accelerometer; 0+acc_avg_num[000]=No averaging; pass sample without filtering
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, ACC_CONF_REG, cmd_data, 2);
    _delay_cycles(5000); // wait 1ms for boot time on the sensor, just needed the first time

  
}

int8_t turnoff_gyro(void){
    cmd_data[0] =0x48;  //configuracion por default
    cmd_data[1] =0x00;  //apagado del giroscopo.
    I2C_Master_WriteReg(GYRO_SLAVE_ADDR, GYRO_CONF_REG, cmd_data, 2);
    _delay_cycles(5000);//wait 30ms after manage gyro to deep suspended mode.
   
}

/*CAMBIO. APARENTEMENTE, el BMI323 simpre manda 2 byte basura
despues la tira de datos. Por lo tanto se cambia de 
6->8*/


int8_t get_acc_reading(uint16_t *accx,uint16_t *accy,uint16_t *accz)
{
    I2C_Master_ReadReg(ACC_SLAVE_ADDR, ACC_DATAX_REG, 8);   //read to result register
    _delay_cycles(50000);
    CopyArray(ReceiveBuffer, buffer, 8);
    *accx= (buffer[3]<<8)+ buffer[2]; //Salteo los dummy  //save the LSB+HSB to data
    *accy= (buffer[5]<<8)+ buffer[4];
    *accz=  (buffer[7]<<8)+ buffer[6];
    return buffer[0];
}
/*The unit is in LSB. The conversion from LSB to acceleration (mg) is based on the range settings and
can be calculated as follows (<0x41>: content of the ACC_RANGE register):

Bosch Sensortec | BMI088 Data sheet 22 | 50

Accel_X_in_mg = Accel_X_int16 / 32768 * 1000 * 2^(<0x41> + 1) * 1.5
Accel_Y_in_mg = Accel_Y_int16 / 32768 * 1000 * 2^(<0x41> + 1) * 1.5
Accel_Z_in_mg = Accel_Z_int16 / 32768 * 1000 * 2^(<0x41> + 1) * 1.5
 */





int8_t wakeup_magH(void){
    cmd_data[0] = 0b10001100; //  10001101 :temp comp enable[1]+reboot[0]+softrst[0]+lowpower[0]+ODR[11]+MODE[00]
    I2C_Master_WriteReg(MAG_SLAVE_ADDR, CFG_REG_A, cmd_data, 1);
    _delay_cycles(5000);
    cmd_data[0] = 0b00000011;
    I2C_Master_WriteReg(MAG_SLAVE_ADDR, CFG_REG_B, cmd_data, 1);
    _delay_cycles(5000);

}

int8_t wakeup_magL(void){
    cmd_data[0] =  0b00011101; //  10011101 :temp comp enable[1]+reboot[0]+softrst[0]+lowpower[1]+ODR[11]+MODE[01]
    I2C_Master_WriteReg(MAG_SLAVE_ADDR, CFG_REG_A, cmd_data, 1);
    _delay_cycles(5000);
    cmd_data[0] = 0b00010011;    // 000 OFF_CANC_ONE_SHOT[1]+ INT_on_DataOFF[0]+ Set_FREQ[0] +OFF_CANC[1] + LPF[1]
    I2C_Master_WriteReg(MAG_SLAVE_ADDR, CFG_REG_B, cmd_data, 1);
    _delay_cycles(5000);
}

int8_t turnoff_mag(void){ //datasheet no dice nada sobre apagarlo y prenderlo,
    //pero tiene dos modos, high resolution y low power; lo prendemos de entrada en low power.
    //cmd_data[0] =0x0C;
    //I2C_Master_WriteReg(MAG_SLAVE_ADDR, CFG_REG_A, cmd_data, 1);
    //_delay_cycles(10000);
}

int8_t get_mag_reading(uint16_t *magx,uint16_t *magy,uint16_t *magz)
{

    //I2C_Master_ReadReg(MAG_SLAVE_ADDR, MAG_XLSB_REG, 6)  Nose que es el 6
    I2C_Master_ReadReg(0x1E, 0x68, 6);//read to result register
    _delay_cycles(50000);
    CopyArray(ReceiveBuffer, buffer, 6);
    *magx= (buffer[1]<<8)+ buffer[0];
    *magy= (buffer[3]<<8)+ buffer[2];
    *magz= (buffer[5]<<8)+ buffer[4];
    __no_operation();
    return buffer[0];
}

/*
 *
#define LIS2MDL_MAG_LSB 1.5 //!< Sensitivity
#define LIS2MDL_MILLIGAUSS_TO_MICROTESLA  0.1 //!< Conversion rate of Milligauss to Microtesla
event->magnetic.x =(float)raw.x * LIS2MDL_MAG_LSB * LIS2MDL_MILLIGAUSS_TO_MICROTESLA;
event->magnetic.y =(float)raw.y * LIS2MDL_MAG_LSB * LIS2MDL_MILLIGAUSS_TO_MICROTESLA;
event->magnetic.z =(float)raw.z * LIS2MDL_MAG_LSB * LIS2MDL_MILLIGAUSS_TO_MICROTESLA;
*
*/


void wakeup_gyro(void){
    cmd_data[0] = 0x0B;   // LSB: ODR 800Hz, range ±125dps
    cmd_data[1] = 0x40;   // MSB: gyr_mode = normal
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, GYRO_CONF_REG, cmd_data, 2);
    _delay_cycles(400000); //45ms.
    return 0;
}

int8_t get_gyro_reading(uint16_t *gyrx, uint16_t *gyry, uint16_t *gyrz){
    I2C_Master_ReadReg(ACC_SLAVE_ADDR, GYRO_DATAX_REG, 8);  // 2 dummy + 6 datos
    _delay_cycles(50000);
    CopyArray(ReceiveBuffer, buffer, 8);
    *gyrx = (buffer[3]<<8) + buffer[2];
    *gyry = (buffer[5]<<8) + buffer[4];
    *gyrz = (buffer[7]<<8) + buffer[6];
    return 0;
}