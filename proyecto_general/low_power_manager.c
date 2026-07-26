/*
 * low_power_manager.c
 *
 *  Created on: 18 mar. 2024
 *      Author: joaco
 */

#include <msp430.h>

#include "low_power_manager.h"

#include <driverlib/rtc.h>
#include <driverlib/wdt_a.h>
#include <driverlib/gpio.h>
#include <driverlib/pmm.h>
#include <driverlib/cs.h>


void init_timer(int ms){
    WDT_A_hold(WDT_A_BASE);

       //Initialize RTC
       RTC_init(RTC_BASE,
           ms*10,
           RTC_CLOCKPREDIVIDER_1);

       RTC_clearInterrupt(RTC_BASE,
           RTC_OVERFLOW_INTERRUPT_FLAG);

       //Enable interrupt for RTC overflow
       RTC_enableInterrupt(RTC_BASE,
           RTC_OVERFLOW_INTERRUPT);

       //Start RTC Clock with clock source SMCLK
       RTC_start(RTC_BASE, RTC_CLOCKSOURCE_VLOCLK);

}

void enter_lpm() {
    //Enter LPM3 mode with interrupts enabled
    __bis_SR_register(LPM3_bits + GIE);
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(RTC_VECTOR)))
#endif
void RTC_ISR (void)
{
    switch (__even_in_range(RTCIV,2)){
        case 0: break;  //No interrupts
        case 2:         //RTC overflow
            __bic_SR_register_on_exit(LPM3_bits + GIE);
            break;


        default: break;
    }
}



