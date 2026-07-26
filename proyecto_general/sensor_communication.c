#include "sensor_communication.h"

#define ACC_ENABLE_REG  0x7D   //ACC_PWR_CNTRL
#define ACC_PWR_CONF 0x7C
#define ACC_XLSB_REG  0x12

#define GYRO_LPM1 0x11

#define CFG_REG_A 0x60 //mag config
#define CFG_REG_B 0x61 //mag config
#define MAG_XLSB_REG  0x68

extern uint8_t buffer[6] = {0};
extern uint8_t cmd_data[1] = {0};

int8_t wakeup_acc(void){
    
    cmd_data[0] =0x00;  //enter active mode to the acc
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, ACC_PWR_CONF, cmd_data, 1);
    _delay_cycles(5000); // wait 1ms for boot time on the sensor, just needed the first time

    cmd_data[0] =0x04;  //enter normal mode to the acc since acc inits in suspended mode
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, ACC_ENABLE_REG, cmd_data, 1);
    _delay_cycles(5000); // wait for 50ms or 5ms datasheet says both

    cmd_data[0] =0x88;  //enter normal mode to the acc since acc inits in suspended mode
                        // Creo q no estoy en normal mode, si no en osr4.
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, 0x40, cmd_data, 1);   //ACC_CONF 0x40
    //_delay_cycles(5000); // wait for 50ms or 5ms datasheet says both
    /*
     * Se puede configurar el rango: Reg 0x41
     * 0x00(default +-3g)
     * 0x01(default +-6g)
     * 0x02(default +-12g)
     * 0x03(default +-24g)
     */
}

int8_t turnoff_acc(void){
    cmd_data[0] =0x00;  //enter normal mode to the acc since acc inits in suspended mode
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, ACC_ENABLE_REG, cmd_data, 1);
    _delay_cycles(5000); // wait for 50ms or 5ms when change the power mode. Datasheet says both
    cmd_data[0] =0x03;  //enter suspended mode to the acc
    I2C_Master_WriteReg(ACC_SLAVE_ADDR, ACC_PWR_CONF, cmd_data, 1);
    _delay_cycles(5000); // wait for 5ms
}

int8_t turnoff_gyro(void){
    cmd_data[0] =0x20;
    I2C_Master_WriteReg(GYRO_SLAVE_ADDR, GYRO_LPM1, cmd_data, 1);
    _delay_cycles(5000);//wait 30ms after manage gyro to deep suspended mode.
    //Deep suspend mode is entered by writing 0x20 to the register GYRO_LPM1. It can be left by writing
    //0x00 to GYRO_LPM1 or by a soft reset (see 4.8).
}

int8_t get_acc_reading(uint16_t *accx,uint16_t *accy,uint16_t *accz)
{
    I2C_Master_ReadReg(ACC_SLAVE_ADDR, ACC_XLSB_REG, 6);   //read to result register
    _delay_cycles(10000);
    CopyArray(ReceiveBuffer, buffer, 6);
    *accx= (buffer[1]<<8)+ buffer[0];   //save the LSB+HSB to data
    *accy= (buffer[3]<<8)+ buffer[2];
    *accz=  (buffer[5]<<8)+ buffer[4];
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
    cmd_data[0] = 0b10001100; //  10001101 :temp comp enable[1]+reboot[0]+softrst[0]+lowpower[0]+ODR[11]+MODE[01]
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
    cmd_data[0] = 0b00010011;
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

    I2C_Master_ReadReg(0x1E, 0x68, 6);//read to result register
    _delay_cycles(10000);
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
