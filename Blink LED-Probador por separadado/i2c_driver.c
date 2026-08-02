#include "i2c_driver.h"
 
uint8_t ReceiveBuffer[MAX_BUFFER_SIZE] = {0};
 
void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t i;
    for (i = 0; i < count; i++)
    {
        dest[i] = source[i];
    }
}
 
/* =========================================================================
 * I2C_Master_WriteReg — polling, sin interrupciones
 * =========================================================================
 */
I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    uint8_t i;
 
    UCB0I2CSA = dev_addr;
 
    /* Start + TX mode */
    UCB0CTLW0 |= UCTR + UCTXSTT;
 
    /* Esperar que el start se envíe */
    while (UCB0CTLW0 & UCTXSTT);
 
    /* Mandar dirección del registro */
    UCB0TXBUF = reg_addr;
    while (!(UCB0IFG & UCTXIFG));
 
    /* Mandar datos */
    for (i = 0; i < count; i++)
    {
        UCB0TXBUF = reg_data[i];
        while (!(UCB0IFG & UCTXIFG));
    }
 
    /* Stop */
    UCB0CTLW0 |= UCTXSTP;
    while (UCB0CTLW0 & UCTXSTP);
 
    return IDLE_MODE;
}
 
/* =========================================================================
 * I2C_Master_ReadReg — polling, sin interrupciones
 * =========================================================================
 */
I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count)
{
    uint8_t i;

    
    UCB0I2CSA = dev_addr;
 
    /* --- Fase TX: mandar dirección del registro --- */
    UCB0CTLW0 |= UCTR + UCTXSTT;
    while (UCB0CTLW0 & UCTXSTT);
 
    UCB0TXBUF = reg_addr;
    while (!(UCB0IFG & UCTXIFG));
 
    /* --- Fase RX: repeated start en modo recepción --- */
    UCB0CTLW0 &= ~UCTR;        /* cambiar a RX */
    UCB0CTLW0 |= UCTXSTT;      /* repeated start */
 
    /* Esperar repeated start */
    while (UCB0CTLW0 & UCTXSTT);
 
    /* Leer bytes */
    for (i = 0; i < count; i++)
    {
        if (i == count - 1)
        {
            /* Antes del último byte mandar STOP */
            UCB0CTLW0 |= UCTXSTP;
        }
        while (!(UCB0IFG & UCRXIFG));
        ReceiveBuffer[i] = UCB0RXBUF;
    }
 
    /* Esperar que el STOP se complete */
    while (UCB0CTLW0 & UCTXSTP);
 
    return IDLE_MODE;
}
 


