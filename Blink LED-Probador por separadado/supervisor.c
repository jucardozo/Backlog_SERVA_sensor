/*
 * supervisor.h
 *
 *  Created on: 11/05/2026
 *      Author: Juan Cardozo
 */

#include <supervisor.h>
#include "driverlib/MSP430FR2xx_4xx/gpio.h"
#include "Board.h"
 /**
 * @brief  Inicializa el GPIO conectado a la salida RESET del LP3470A.
 *
 * Configura el pin como entrada con pull-up interno habilitado (el MSP430FR2476
 * tiene resistores internos de ~20-50kΩ; para mayor robustez se recomienda
 * un pull-up externo de 10kΩ al nivel lógico correcto después del divisor).
 *
 *  Puerto 3 pin 5 => dedicado para el supervisor.
 */
 void init_supervisor(void){
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_BSTATUS,GPIO_PIN_BSTATUS);
    return;
 }


/**
 * @brief  Lee el estado actual del pin RESET del LP3470A.
 *
 *         Bateria Baja ? 
 *         si Vcc>3.08v => Reset =1 o 3v3
 *         si Vcc<3.08v => Reset= 0
 * @return devuelve un 1-TRUE si el pin está en LOW (RESET asertado),
 *         devuelve un 0-FALSE   si el pin está en HIGH (tensión nominal).
 *
 *  reset negado = > chequear bien como funciona.
 **/

uint8_t get_batery_status(void){
    return !GPIO_getInputPinValue(GPIO_PORT_BSTATUS,GPIO_PIN_BSTATUS); // si Vcc>308 devuelvo 0, Bateria baja? NO = 0
}
