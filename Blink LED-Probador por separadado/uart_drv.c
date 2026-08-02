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

/******************************************************/
    /*Configuracion para que la salida sea P2.5 P2.6*/
    /*
    P2SEL0 |= BIT5 | BIT6;
    P2SEL1 &= ~(BIT5 | BIT6);
    UCA1CTLW0 = UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK;

    UCA1BR0 = 52;
    UCA1BR1 = 0;
    UCA1MCTLW = 0x4900 | UCOS16 | UCBRF_1;

    UCA1CTLW0 &= ~UCSWRST;
    UCA1IE |= UCRXIE;*/   

/******************************************************/

    //Configuracion para que la salida sea P1.4 (TX) y P1.5(RX)
    P1SEL0 |= BIT4 | BIT5;
    P1SEL1 &= ~(BIT4 | BIT5);

    // Configure UART
    //UCA0CTLW0 |= UCSWRST; //Software reset enable
    //UCA0CTLW0 |= UCSSEL__SMCLK;  //A clock source select. These bits select the BRCLK source clock.

    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 14-4: UCBRSx = 0x49
    // UCBRFx = int ( (52.083-52)*16) = 1
    //UCA0BR0 = 52;                             // 8000000/16/9600
    //UCA0BR1 = 0x00;
    //UCA0MCTLW = 0x4900 | UCOS16 | UCBRF_1;
  
    UCA0CTLW0 = UCSWRST;
    UCA0CTLW0 |= UCSSEL__SMCLK;

    UCA0BR0 = 52;
    UCA0BR1 = 0;
    UCA0MCTLW = 0x4900 | UCOS16 | UCBRF_1;

    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE |= UCRXIE;               //Habilito interrupciones de RX.
   

    //UCA1CTLW0 |= UCPEN;
    //UCA1CTLW0 |= UCPAR;


    //UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    //UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    return;
}

void send_msg(char* message, int message_len){

    int i;
    for(i=0; i<message_len;i++){
        pop_in_data(&TXbuffer, message[i]);
    }

   // UCA0IE |= UCTXIE;
   UCA0IFG |= UCTXIFG;  // forzar flag
   UCA0IE |= UCTXIE;   //Habilito interrupciones 
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

/*
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
        aux_rx = UCA0RXBUF;
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
}*/

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR(void)
#else
#error Compiler not supported!
#endif
{
    unsigned char aux_rx = 0;
    unsigned char aux_tx = 0;

    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE:
            break;

        case USCI_UART_UCRXIFG:
            // Se recibió un byte
            aux_rx = UCA0RXBUF;
            pop_in_data(&RXbuffer, aux_rx);
            break;

        case USCI_UART_UCTXIFG:

             if (TXbuffer.distance < BUFSIZE)  //Si hya datos transmito
                {
                aux_tx = pop_out_data(&TXbuffer);
                UCA0TXBUF = aux_tx;
                }
            else                //si ya no hay datos, deshailito las interrupciones
                {
                UCA0IE &= ~UCTXIE;
                }
            break;

        case USCI_UART_UCSTTIFG:
            break;

        case USCI_UART_UCTXCPTIFG:
            break;

        default:
            break;
    }
}

