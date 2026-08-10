/*
 * rak3172.c
 *
 * Driver para RAK3172 en modo P2P
 * Frecuencia: 915MHz (Argentina)
 * UART: UCA0 - P1.4/P1.5 - 9600 baud
 */

#include "RAK_driver.h"
#include "uart_drv.h"
#include <string.h>
#include <msp430.h>



/********************************PRIVADAS******************************************************** */
static bool rak_cmd_expect(char* cmd, char* expected);


/* =========================================================================
 * Helper interno — manda comando y espera respuesta
 * Bloqueante con timeout por iteraciones
 * =========================================================================
 */
static bool rak_cmd_expect(char* cmd, char* expected)
{
    // 1. Mandar comando
 
    if(cmd != 0){
    send_msg(cmd, strlen(cmd));
    }
    
    // 2. Esperar respuesta con timeout
    char response[64] = {0};
    uint8_t idx = 0;
    uint32_t timeout = 200000;  // ~1 segundo a 8MHz

    while(timeout--)
    {
        if(!rx_buffer_empty())
        {
            char c = get_last_rx_byte();
            response[idx++] = c;

            // verificar si llegó el string esperado
            if(strstr(response, expected))
            {
                return true;
            }

            // verificar error
            if(strstr(response, "ERROR"))
            {
                return false;
            }

            if(idx >= 63) idx = 0;  // evitar overflow
        }
    }
    return false;  // timeout
}

/* =========================================================================
 * rak3172_init
 * =========================================================================
 */
void rak3172_init(void)
{
    send_msg("AT+ATM\r\n", 8);
     __delay_cycles(8000000);
    send_msg("AT+NWM=0\r\n", 10);
    __delay_cycles(8000000);

    send_msg("AT+PRECV=0\r\n", 12);
    __delay_cycles(800000);

    send_msg("AT+PFREQ=915000000\r\n", 20);
    __delay_cycles(800000);

    send_msg("AT+PSF=7\r\n", 10);
    __delay_cycles(800000);

    send_msg("AT+PBW=0\r\n", 10);
    __delay_cycles(800000);

    send_msg("AT+PCR=0\r\n", 10);
    __delay_cycles(800000);

    send_msg("AT+PTP=14\r\n", 11);
    __delay_cycles(800000);
}

/* =========================================================================
 * rak3172_send
 * Convierte el byte de estado a hex string y lo manda por LoRa P2P
 * Ejemplo: 0x1C → "AT+PSEND=1C"
 * =========================================================================
 */
void rak3172_send(uint8_t status_byte)
{
    char payload[3];
    payload[0] = "0123456789ABCDEF"[(status_byte >> 4) & 0xF];
    payload[1] = "0123456789ABCDEF"[status_byte & 0xF];
    payload[2] = '\0';

    // Armar y mandar comando
    send_msg("AT+PSEND=", 9);
    send_msg(payload, 2);
    send_msg("\r\n", 2);

    // Esperar confirmacion de transmision
    rak_cmd_expect(0, "OK");
}

/* =========================================================================
 * rak3172_wakeup
 * =========================================================================
 */
void rak3172_wakeup(void)
{
    rak_cmd_expect("AT", "OK");
    _delay_cycles(800000);
}

/* =========================================================================
 * rak3172_sleep
 * Parametro: tiempo en ms que duerme el RAK
 * 0 = sleep indefinido hasta recibir byte por UART
 * =========================================================================
 */
void rak3172_sleep(void)
{
    rak_cmd_expect("AT+SLEEP=0", "OK");
    _delay_cycles(800000);
}