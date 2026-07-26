/*
 * supervisor.h
 *
 *  Created on: 11/05/2026
 *      Author: Juan Cardozo
 */

#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_

#include <stdint.h>

 /**
 * @brief  Inicializa el GPIO conectado a la salida RESET del LP3470A.
 *
 * Configura el pin como entrada con pull-up interno habilitado (el MSP430FR2476
 * tiene resistores internos de ~20-50kΩ; para mayor robustez se recomienda
 * un pull-up externo de 10kΩ al nivel lógico correcto después del divisor).
 *
 *  Puerto 3 pin 5 => dedicado para el supervisor.
 */
void init_supervisor(void);

/**
 * @brief  Lee el estado actual del pin RESET del LP3470A.
 *
 *         Bateria Baja ? 
 * @return devuelve un 1-TRUE si el pin está en LOW (RESET asertado),
 *         devuelve un 0-FALSE   si el pin está en HIGH (tensión nominal).
 *
 *
 **/
uint8_t get_status(void);

#endif // SUPERVISOR_H_
