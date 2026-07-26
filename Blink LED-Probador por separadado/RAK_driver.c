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
    if(cmd != 0){
        send_msg(cmd, strlen(cmd));
        send_msg("\r\n", 2);
        _delay_cycles(800000);  // dar tiempo al ISR para transmitir
    }

    char response[64] = {0};
    uint8_t idx = 0;
    uint32_t timeout = 8000000;

    while(timeout--)
    {
        if(!rx_buffer_empty())
        {
            char c = get_last_rx_byte();
            response[idx++] = c;

            if(strstr(response, expected)) return true;
            if(strstr(response, "ERROR")) return false;
            if(idx >= 63) idx = 0;
        }
    }
    return false;
}

/* =========================================================================
 * rak3172_init
 * =========================================================================
 */
void rak3172_init(void)
{
    bool result;
    _delay_cycles(800000);  // esperar que el RAK arranque

    // Verificar comunicacion
  result=  rak_cmd_expect("AT", "OK");
    _delay_cycles(800000);
   

    // Modo P2P
    rak_cmd_expect("AT+NWM=0", "RAK3172");
    _delay_cycles(800000);

    // Frecuencia 915MHz
    rak_cmd_expect("AT+PFREQ=915000000", "OK");
    _delay_cycles(800000);

    // Spreading Factor 7
    rak_cmd_expect("AT+PSF=7", "OK");
    _delay_cycles(800000);

    // Bandwidth 125kHz
    rak_cmd_expect("AT+PBW=0", "OK");
    _delay_cycles(800000);

    // Code Rate 4/5
    rak_cmd_expect("AT+PCR=0", "OK");
    _delay_cycles(800000);

    // TX Power 14dBm
    rak_cmd_expect("AT+PTP=14", "OK");
    _delay_cycles(800000);
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