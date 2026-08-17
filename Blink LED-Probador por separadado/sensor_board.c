/*
 * sensor_board.c
 *
 *  Created on: 20 abr. 2024
 *      Author: joaco
 */

#include <msp430.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "i2c_driver.h"
#include "sensor_communication.h"
#include "uart_drv.h"
#include "sensor_board.h"

/* Local Function Prototypes */
void initUart();
void initClockTo1MHz();
void initClockTo8MHz();
void initGPIO();
void initI2C();
void uint16_to_str(uint16_t value, char *buffer, int buffer_size);
void Software_Trim();                       // Software Trim to get the best DCOFTRIM value


/* global parameters */


void uint16_to_str(uint16_t val, char *buffer, int buffer_size) {
    int16_t value = val;
        // Asegurarse de que el buffer sea lo suficientemente grande para contener el n mero
        if (buffer_size < 7 ) { // m ximo 4 d gitos para un n mero uint16_t m s el terminador nulo
            return; // O manejar el error de alguna manera
        }

        if(value>=0){
            buffer[0]='+';
        }
        else {
            buffer[0]='-';
            value = -val;

        }
        // Convertir el valor a una cadena de caracteres manualmente
        int i = 1;
        do {
            buffer[i++] = value % 10 + '0';
            value /= 10;
        } while (value != 0);

        // Agregar el terminador nulo
        //buffer[i] = '\n';
        //buffer[i+1] = '\0';

        // Invertir la cadena de caracteres, ya que se gener  en orden inverso
        int start = 1;
        int end = i - 1;
        while (start < end) {
            char temp = buffer[start];
            buffer[start] = buffer[end];
            buffer[end] = temp;
            start++;
            end--;
        }
}





/**
* Configure UART (see UG)
* 9600 baud
 */
void initUart()
{
    // Configure UART 9600 baud
    UCA1CTLW0 |= UCSWRST;                  // eUSCI_A logic held in reset state.
    UCA1CTLW0 |= UCSSEL__SMCLK; // One stop bit, no parity, LSB first, 8-bit data as the default setting

    UCA1BR0 = 6;                                              // 1000000/16/9600
    UCA1BR1 = 0x00;
    UCA1MCTLW = 0x2000 | UCOS16 | UCBRF_8;

    UCA1CTLW0 &= ~UCSWRST;               // eUSCI_A reset released for operation
    UCA1IE = UCRXIE;                              // Enable USCI_A0 RX interrupt
}

void initGPIO()
{
    // I2C pins
    P1SEL0 |= BIT2 | BIT3;
    P1SEL1 &= ~(BIT2 | BIT3);

    // Configure UART pins
    P2SEL0 |= BIT6 | BIT5;                    // set 2-UART pin as second function

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

void initI2C()
{

    UCB0CTLW0 = UCSWRST;                      // Enable SW reset
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK | UCSYNC; // I2C master mode, SMCLK
    UCB0BRW = 80;                            // fSCL = SMCLK/10 = ~100kHz
//    UCB0I2CSA = SLAVE_ADDR;                   // Slave Address
    UCB0CTLW0 &= ~UCSWRST;                    // Clear SW reset, resume operation
   // UCB0IE |= UCNACKIE;
}

void Software_Trim()
{
    unsigned int oldDcoTap = 0xffff;
    unsigned int newDcoTap = 0xffff;
    unsigned int newDcoDelta = 0xffff;
    unsigned int bestDcoDelta = 0xffff;
    unsigned int csCtl0Copy = 0;
    unsigned int csCtl1Copy = 0;
    unsigned int csCtl0Read = 0;
    unsigned int csCtl1Read = 0;
    unsigned int dcoFreqTrim = 3;
    unsigned char endLoop = 0;

    do
    {
        CSCTL0 = 0x100;                         // DCO Tap = 256
        do
        {
            CSCTL7 &= ~DCOFFG;                  // Clear DCO fault flag
        }while (CSCTL7 & DCOFFG);               // Test DCO fault flag

        __delay_cycles((unsigned int)3000 * 1);// Wait FLL lock status (FLLUNLOCK) to be stable
                                                           // Suggest to wait 24 cycles of divided FLL reference clock
        while((CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)) && ((CSCTL7 & DCOFFG) == 0));

        csCtl0Read = CSCTL0;                   // Read CSCTL0
        csCtl1Read = CSCTL1;                   // Read CSCTL1

        oldDcoTap = newDcoTap;                 // Record DCOTAP value of last time
        newDcoTap = csCtl0Read & 0x01ff;       // Get DCOTAP value of this time
        dcoFreqTrim = (csCtl1Read & 0x0070)>>4;// Get DCOFTRIM value

        if(newDcoTap < 256)                    // DCOTAP < 256
        {
            newDcoDelta = 256 - newDcoTap;     // Delta value between DCPTAP and 256
            if((oldDcoTap != 0xffff) && (oldDcoTap >= 256)) // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim--;
                CSCTL1 = (csCtl1Read & (~(DCOFTRIM0+DCOFTRIM1+DCOFTRIM2))) | (dcoFreqTrim<<4);
            }
        }
        else                                   // DCOTAP >= 256
        {
            newDcoDelta = newDcoTap - 256;     // Delta value between DCPTAP and 256
            if(oldDcoTap < 256)                // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim++;
                CSCTL1 = (csCtl1Read & (~(DCOFTRIM0+DCOFTRIM1+DCOFTRIM2))) | (dcoFreqTrim<<4);
            }
        }

        if(newDcoDelta < bestDcoDelta)         // Record DCOTAP closest to 256
        {
            csCtl0Copy = csCtl0Read;
            csCtl1Copy = csCtl1Read;
            bestDcoDelta = newDcoDelta;
        }

    }while(endLoop == 0);                      // Poll until endLoop == 1

    CSCTL0 = csCtl0Copy;                       // Reload locked DCOTAP
    CSCTL1 = csCtl1Copy;                       // Reload locked DCOFTRIM
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked
}


void init_magacc_driver(){
    initGPIO();
    init_uart_drv();
    initI2C();
    wakeup_magH();
    wakeup_acc();
    wakeup_gyro();
  
}

void reinit_magacc_driver(){
    initGPIO();
    initI2C();
    __bis_SR_register(GIE);  // Enable global interrupts
    wakeup_magH();
    wakeup_acc();
}

void read_mag(int16_t* magx,int16_t* magy,int16_t* magz){
    get_mag_reading(magx,magy,magz);
}

void read_acc(int16_t* accx,int16_t* accy,int16_t* accz){
    get_acc_reading(accx,accy,accz);
}

void read_gyro(int16_t* gyrx, int16_t* gyry, int16_t* gyrz){
    get_gyro_reading(gyrx, gyry, gyrz);
}