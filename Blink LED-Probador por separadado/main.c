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

//Objetivo: BlinkTest sirve para  chequear que serva podia ser programada.
//#define BlinkTest

//Objetivo: Verificar comunicacion con el magnetometro. Usar mismo archivos que en el Serva.
//#define SensoresTest

//Objetivo, verificar comunicacion UART , MSP430 con RAK
//#define RakTest

//Objetico, Verificar solo Comunicacion UART
#define UartTest


#include <driverlib.h>
#include "sensor_board.h"
#include "RAK_driver.h"
#include "uart_drv.h"



void initClockTo8MHz(void);



int main(void) {


    #ifdef BlinkTest

    volatile uint32_t i;

    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // Set P4.7 to output direction
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN1
        );

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

    while(1)
    {
        // Toggle P4.7 output (blue LED)
        GPIO_toggleOutputOnPin(
            GPIO_PORT_P1,
            GPIO_PIN1
            );

        // Delay
        for(i=10000; i>0; i--);
    }

    #endif  // Fin Codigo Blink Test

    
    #ifdef SensoresTest

     // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // init normal del sistema
    init_magacc_driver();

    
    //init_uart_drv();
    

    int16_t magx, magy, magz;
    int16_t accx, accy, accz;
    volatile uint32_t i;
    
    while(1){
        read_mag(&magx, &magy, &magz);
        //Delay
        for(i=10000; i>0; i--); 
        read_acc(&accx,&accy, &accz);
        //Delay
        for(i=10000; i>0; i--);     
    }
    #endif // Fin codigo Sensores

 #ifdef RakTest
    WDT_A_hold(WDT_A_BASE);
    initClockTo8MHz();          // ← falta esto
    PMM_unlockLPM5();
    init_uart_drv();
  //  __bis_SR_register(GIE);
    _delay_cycles(1600000);

    rak3172_init();
    rak3172_send(0x1C);

while(1);


    #endif // Fin codigo Sensores


 #ifdef UartTest

WDT_A_hold(WDT_A_BASE);

initClockTo8MHz();
PMM_unlockLPM5();
init_uart_drv();

__delay_cycles(800000);

__bis_SR_register(GIE);

send_msg("Hola Mundo\r\n", 12);

while(1);


/*ANDuvooo UART
char msg[] = "Hola Mundo\r\n";
int i=0;

for ( i=0 ; msg[i] != '\0'; i++)
{
    while (!(UCA1IFG & UCTXIFG));   // Esperar que TXBUF esté libre
    UCA1TXBUF = msg[i];
}

while (!(UCA1IFG & UCTXCPTIFG));    // Esperar que termine el último bit

while(1);*/

#endif

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