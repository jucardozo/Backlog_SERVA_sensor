/*
 * uart_drv.c
 *
 *  Created on: 17 ene. 2024
 *      Author: joaco
 */



/*
*Se cambio la definiciones de los pines. se paso de 2-uart UCA1 a uno que es Puerto 1-UCA0
*
*
*
*/
#include <msp430.h>
#include "uart_drv.h"
#include "circular_buffer.h"

/*Buffers*/

static circular_buffer_t RXbuffer;
static circular_buffer_t TXbuffer;

/*Definiciones de funciones*/

void init_uart_drv(){

    init_circular_buffer(&RXbuffer);
    init_circular_buffer(&TXbuffer);
    // Configure UART pins
   P1SEL0 |= BIT4 | BIT5;                    // set 2-UART pin as second function

    // Configure UART
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;

    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 14-4: UCBRSx = 0x49
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BR0 = 52;                             // 8000000/16/9600
    UCA0BR1 = 0x00;
    UCA0MCTLW = 0x4900 | UCOS16 | UCBRF_1;

    //UCA1CTLW0 |= UCPEN;
    //UCA1CTLW0 |= UCPAR;


    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    return;
}

void send_msg(char* message, int message_len){

    int i;
    for(i=0; i<message_len;i++){
        pop_in_data(&TXbuffer, message[i]);
    }

    UCA0IE |= UCTXIE;
}

char rx_buffer_empty(){
    if(RXbuffer.distance == BUFSIZE){
        return 1;
    }
    else return 0;
}

char get_last_rx_byte(){
   return pop_out_data(&RXbuffer);
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  unsigned char aux_rx = 0;
  unsigned char aux_tx = 0;
  switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        aux_rx = UCA1RXBUF;
        pop_in_data(&RXbuffer, aux_rx);
        __no_operation();
        break;
    case USCI_UART_UCTXIFG:
        // Transmit the byte
        aux_tx = pop_out_data(&TXbuffer);
        UCA0TXBUF = aux_tx;
        // If last byte sent, disable the interrupt
        if((TXbuffer.distance) >= BUFSIZE)
        {
            UCA0IE &= ~UCTXIE;
        }
        break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}



