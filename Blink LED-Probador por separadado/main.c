/* --COPYRIGHT--,BSD
 * Copyright (c) 2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//***************************************************************************************
//  Blink the LED Demo - Software Toggle P4.7
//
//  Description; Toggle P4.7 inside of a software loop using DriverLib.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430FR2476
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P4.7|-->LED (blue)
//
//  E. Chen
//  Texas Instruments, Inc
//  Feb 2019
//  Built with Code Composer Studio v8
//***************************************************************************************


#include <msp430.h>
#include <driverlib.h>
#include "sensor_board.h"
#include "uart_drv.h"
#include "uart_drv.h"
#include "supervisor.h"



/***************************/
/* FUNCTION DECLARATIONS   */
/***************************/

void initClockTo8MHz(void);
bool collect_sample(int16_t*readingval);
void send_status(bool isturning);

#define COUNT_TOTAL     10    // Cantidad de muestra que toma
#define THRESHOLD       400    /* ajustar empíricamente en campo */

/* Bits del byte de estado */
/* 000X XX00*/
#define BIT_POWER        (1 << 4)
#define BIT_BATERIA_BAJA (1 << 3)
#define BIT_GIRO         (1 << 2)

/*
    0b 0001 0000 = 0x10
BIT_POWER = 1 → bit 4
BIT_BATERIA_BAJA = 0 → batería OK
BIT_GIRO = 0 → no gira

    0b 0001 0100 = 0x14
BIT_POWER = 1 → bit 4
BIT_BATERIA_BAJA = 0 → batería OK
BIT_GIRO = 1 → no gira

   0b 0001 1000 = 0x18
BIT_POWER = 1 → bit 4
BIT_BATERIA_BAJA = 1 → batería baja
BIT_GIRO = 0 → no gira

*/

uint8_t Serva_Status = 0;



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    initClockTo8MHz();
    init_magacc_driver();
    init_uart_drv();
    __bis_SR_register(GIE);

    int16_t readingval = 0;

    while(1)
    {
        if(collect_sample(&readingval))
        {
            /* Valor absoluto del promedio */
            int16_t abs_val = readingval < 0 ? -readingval : readingval;

            /* Comparar con umbral */
            bool isturning = (abs_val > THRESHOLD);

            /* Mandar status */
            send_status(isturning);
        }
        _delay_cycles(50000);  // ~10ms entre muestras
    }
}

void initClockTo8MHz(void)              //Activo el clock de 8MHz
{
    __bis_SR_register(SCG0);                 // disable FLL
     CSCTL3 |= SELREF__REFOCLK;               // Set REFO as FLL reference source
     CSCTL0 = 0;                              // clear DCO and MOD registers
     CSCTL1 &= ~(DCORSEL_7);                  // Clear DCO frequency select bits first
     CSCTL1 |= DCORSEL_3;                     // Set DCO = 8MHz
     CSCTL2 = FLLD_0 + 243;                   // DCODIV = 8MHz
     __delay_cycles(3);
     __bic_SR_register(SCG0);                 // enable FLL
     while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked
     CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                              // default DCODIV as MCLK and SMCLK source
}

bool collect_sample(int16_t *readingval) //va devolver el valor medio.
{
    static int16_t count = 0;
    static int32_t suma  = 0;

    int16_t accx = 0;
    int16_t accy = 0;
    int16_t accz = 0;

    read_acc(&accx, &accy, &accz);

    suma = suma + (int32_t)accx;  // acumulamos X
    count += 1;

    bool retval = false;
    if (count == COUNT_TOTAL)
    {
        count = 0;
        int32_t tentative_readingval = suma / COUNT_TOTAL;
        suma = 0;
        *readingval = (int16_t) tentative_readingval;
        retval = true;
    }
    return retval;
}

void send_status(bool isturning)
{
    Serva_Status = BIT_POWER;
    if(isturning)            Serva_Status |= BIT_GIRO;
    if(get_batery_status())  Serva_Status |= BIT_BATERIA_BAJA;

    char payload[3];
    payload[0] = "0123456789ABCDEF"[(Serva_Status >> 4) & 0xF];
    payload[1] = "0123456789ABCDEF"[Serva_Status & 0xF];
    payload[2] = '\0';

    send_msg("AT+PSEND=", 9);
    send_msg(payload, 2);
    send_msg("\r\n", 2);
}