#include <msp430.h>
#include <stdint.h>
#include <SPI.h>
//#include <./HAL_BOARD/HAL_BOARD.h>
//#include <./HAL_MCU/HAL_MCU.h>

void SPI_Master_Mode_Init(void) {
   // switch (eUSCI) {
    /*
     *     UCA0CTLW0 |= UCCKPL | UCMSB | UCMST | UCMODE_0 | UCSYNC; //3-pin, 8-bit SPI master
    //Clock polarity select - The inactive state is high
    //MSB first
    UCA0CTLW0 |= UCSSEL_2;                          //Use SMCLK, keep RESET
    UCA0BR0 = 3;                             //Initial SPI clock must be <400kHz
    UCA0BR1 = 0;                                //f_UCxCLK = 1MHz/(3+1) = 250kHz
    UCA0CTLW0 &= ~UCSWRST;
    */



   // case eUSCI_A0: // SD-CARD
        P1SEL0 |= BIT4 | BIT5 | BIT6 | BIT7;      // set 4-SPI pin as second function
        UCA0CTLW0 |= UCSWRST;    // Software reset enabled (Puts eUSCI in configuration mode)
        UCA0CTLW0 |=
        UCMODE_0 |  // 3-pin SPI mode (without slave transmission control a.k.a. UCxSTE pin)
                UCMST  |  // Master mode
                UCSYNC |  // Synchronous mode
                UCCKPH |  // Clock phase = Data is captured on the first UCLK edge and changed on the following edge
                UCCKPL |  // Clock polarity = LOW as inactive state
                UCSSEL_2 |  // eUSCI clock source = SMCLK
                UCMSB;      // Direction of the receive and transmit shift register is MSB first
        UCA0BR0 = 3;
        UCA0BR1 = 0;
       // UCA0BRW = 8;             // Clock prescaler = SMCLK/64 = 250kHz => SMCLK 1MHz

        UCA0CTLW0 &= ~UCSWRST;    // Clear software reset
        UCA0IE |= UCRXIE;   // Enable USCI_A0 RX interrupt
        return;

}

static void rcvr_spi_m(uint8_t *dst) {
    *dst = rcvr_spi();
    return;
}

// Receive a byte from MMC via SPI  (Platform dependent)
static uint8_t rcvr_spi(void) {
    uint8_t ui8RcvDat;				// Receive variable

    uint16_t gie = __get_SR_register() & GIE;	// Save interrupt state
    __disable_interrupt();

    UCA0IFG &= ~UCRXIFG;				// Ensure RXIFG clear
    while (!(UCA0IFG & UCTXIFG));			// Wait for TX ready
    UCA0TXBUF = 0xFF;				// Send dummy byte
    while (!(UCA0IFG & UCRXIFG));			// Wait for RX buffer

    ui8RcvDat = UCA0RXBUF;				// Read RX buffer

    __bis_SR_register(gie);				// Reload interrupt state

    return (uint8_t) ui8RcvDat;

}

// Transmit a byte to MMC via SPI  (Platform dependent)                 
 void xmit_spi(uint8_t dat) {
    uint16_t gie = __get_SR_register() & GIE;	// Save interrupt state
    __disable_interrupt();				// Disable interrupts

    while (!(UCA0IFG & UCTXIFG));			// Wait for TX ready Flag de interrupcion
    UCA0TXBUF = dat;				// Write byte
    while (UCA0STATW & UCBUSY);

    UCA0RXBUF;					// Read to empty RX buffer, clear any ovverrun

    __bis_SR_register(gie);				// Reload interrupt state
    return;
}
