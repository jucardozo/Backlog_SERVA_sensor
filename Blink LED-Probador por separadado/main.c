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
#include "i2c_driver.h"
#include "supervisor.h"
#include "low_power_manager.h"
#include "auxfuncs.h"



/***************************/
/* FUNCTION DECLARATIONS   */
/***************************/

void initClockTo8MHz(void);
bool collect_sample(int16_t *readingval_acc, int16_t *readingval_mag);
void send_status(bool isturning,bool ismoving);
bool is_rf_time();


/********************DEFINES**************************** */
#define COUNT_TOTAL     10    // Cantidad de muestra que toma

#define REAL_MINUTES_TILL_RF  1        // cambiar a 60 para producción
#define CYCLES_TILL_RF        REAL_MINUTES_TILL_RF * (60 / COUNT_TOTAL)
#define SEC_COUNT 5                    //Tiempo de instalacion . WARM_up



#define THRESHOLD_ACC       25    /* ajustar empíricamente en campo */
#define THRESHOLD_MAG       25    /* ajustar empíricamente en campo */

#define BUFFERLENRAW        128
#define ENVELOPEBUFFERLEN   10   // antes 60

/* Bits del byte de estado */
/* 000X XX00*/
#define BIT_POWER        (1 << 4)
#define BIT_BATERIA_BAJA (1 << 3)
#define BIT_GIRO         (1 << 2)   // depende del mag.
#define BIT_MOVIMIENTO   (1 << 1)   // depende del acc.

/*
 * Combinaciones posibles:
 * 0x10 = xxx1 000x → Power ON, bat OK,  no gira, no bombea
 * 0x12 = xxx1 001x → Power ON, bat OK,  no gira, bombea
 * 0x16 = xxx1 011x → Power ON, bat OK,  gira,    bombea
 * 0x18 = xxx1 100x → Power ON, bat BAJA, no gira, no bombea
 * 0x1A = xxx1 101x → Power ON, bat BAJA, no gira, bombea
 * 0x1E = xxx1 111x → Power ON, bat BAJA, gira,    bombea

 *Por como esta la instalacino creo q no es posible xq para que gire tiene que bombear.
 *0x1C = xxx1 110x → Power ON, bat BAJA, gira,    no bombea
 *0x14 = xxx1 010x → Power ON, bat OK,  gira,    no bombea 
 */



uint8_t Serva_Status = 0;



int16_t last_acc_readings[BUFFERLENRAW]         = {0};
int16_t last_acc_readings_envelope[ENVELOPEBUFFERLEN] = {0};
int16_t last_mag_readings[BUFFERLENRAW]         = {0};
int16_t last_mag_readings_envelope[ENVELOPEBUFFERLEN] = {0};



/*******************/
/*******MAIN*******  /
/*******************/

int main(void)
{
    
    WDTCTL = WDTPW | WDTHOLD;
    initClockTo8MHz();
    init_magacc_driver();

 
    __bis_SR_register(GIE);

    rak3172_init();


    /* Inicializar RTC para despertar cada ~1 segundo */
    init_timer(400); //800 para que se despierte a 1 segundo


    int second_count = SEC_COUNT;
    int minutes_to_begin = 1; 
    int cycles_to_begin = minutes_to_begin * 60 / second_count;


    //Tiempo para la instalacion. 1 minuto.
    while((cycles_to_begin--) > 0)
    {
        send_status(true, false);  // keep alive -- 0X14
    
        while((second_count--) > 0)
        {
            enter_lpm();
        }
    second_count = SEC_COUNT;
    }


    /* Precargar buffers con primera medición */
    int16_t ax, ay, az, mx, my, mz;
    read_acc(&ax, &ay, &az);
    read_mag(&mx, &my, &mz);

    int i;
    for(i = 0; i < BUFFERLENRAW; i++)
    {
        last_acc_readings[i] = ax;
        last_mag_readings[i] = my;
    }

    int16_t readingval_acc = 0;
    int16_t readingval_mag = 0;


/********************MAIN LOOP************************************************** */

    while(1)
    {
        /*collect_sample toma COUNT_TOTAL muestras de mag y acc y devuelve true con el promedio de cada uno*/
        if(collect_sample(&readingval_acc, &readingval_mag))
        {
            /* --- Acelerómetro con media móvil --- */
           
            append_and_shift(last_acc_readings, BUFFERLENRAW, readingval_acc); //Guarda el promedio en Last_acc_reading 
            int16_t mean_corrected_acc = readingval_acc - get_mean(last_acc_readings, BUFFERLENRAW); // Sacamos la media de los ultimos promedios y lo restamos al promedio de la ultima medicion
            if(mean_corrected_acc < 0) mean_corrected_acc = -mean_corrected_acc; //Sacamos el valor absoluto
            append_and_shift(last_acc_readings_envelope, ENVELOPEBUFFERLEN, mean_corrected_acc); //Guardamos el promedio sin el offset q hubo. PRomedio centrado en cero. 
            int16_t result_acc = get_mean(last_acc_readings_envelope, ENVELOPEBUFFERLEN);

            /* Magnetómetro */
            append_and_shift(last_mag_readings, BUFFERLENRAW, readingval_mag);
            int16_t mean_corrected_mag = readingval_mag - get_mean(last_mag_readings, BUFFERLENRAW);
            if(mean_corrected_mag < 0) mean_corrected_mag = -mean_corrected_mag;
            append_and_shift(last_mag_readings_envelope, ENVELOPEBUFFERLEN, mean_corrected_mag);
            int16_t result_mag = get_mean(last_mag_readings_envelope, ENVELOPEBUFFERLEN);

           
            //bool isturning = (result_mag > THRESHOLD_MAG);
            //bool ismoving = (result_acc > THRESHOLD_ACC);
            if(is_rf_time())
            {
                 /* --- Decisión --- */
                bool isturning = (result_mag > THRESHOLD_MAG);
                bool ismoving = (result_acc > THRESHOLD_ACC);
                
                send_status(isturning, ismoving);
            }
           
        }
        //_delay_cycles(50000);  // ~10ms entre muestras
        enter_lpm();  // ← duerme hasta el próximo tick del RTC
    }
}




/*******************/
/* HELPER FUNCTIONS */
/*******************/

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

bool collect_sample(int16_t *readingval_acc, int16_t *readingval_mag)
{
    static int16_t count = 0;
    static int32_t suma_acc = 0;
    static int32_t suma_mag = 0;

    int16_t accx = 0, accy = 0, accz = 0;
    int16_t magx = 0, magy = 0, magz = 0;

    read_acc(&accx, &accy, &accz);
    read_mag(&magx, &magy, &magz);

    suma_acc = suma_acc + (int32_t)accx;
    suma_mag = suma_mag + (int32_t)magy;  
    count += 1;

    bool retval = false;
    if(count == COUNT_TOTAL)
    {
        count = 0;
        *readingval_acc = (int16_t)(suma_acc / COUNT_TOTAL);
        *readingval_mag = (int16_t)(suma_mag / COUNT_TOTAL);
        suma_acc = 0;
        suma_mag = 0;
        retval = true;
    }
    return retval;
}

void send_status(bool isturning,bool ismoving)
{
    Serva_Status = BIT_POWER;
    if(isturning)
    {
        Serva_Status |= BIT_GIRO;
        Serva_Status |= BIT_MOVIMIENTO;     //Momentaneo. Hasta que vea como puedo ver que este prendida la bomba.
    }

   // if(ismoving)        Serva_Status |= BIT_MOVIMIENTO;
    if(get_batery_status())  Serva_Status |= BIT_BATERIA_BAJA;

    send_msg("AT+PRECV=0\r\n", 12);
    __delay_cycles(800000);
    char full_cmd[14];
    strcpy(full_cmd, "AT+PSEND=");
    full_cmd[9]  = "0123456789ABCDEF"[(Serva_Status >> 4) & 0xF];
    full_cmd[10] = "0123456789ABCDEF"[Serva_Status & 0xF];
    full_cmd[11] = '\r';
    full_cmd[12] = '\n';
    full_cmd[13] = '\0';

    send_msg(full_cmd, 13);
}



bool is_rf_time()
{
    static int count = 0;
    count += 1;
    bool ret = false;
    if (count == CYCLES_TILL_RF)
    {
        ret   = true;
        count = 0;
    }
    return ret;
}

