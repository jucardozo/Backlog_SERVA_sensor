/*
 * rak3172.h
 *
  Author: Juan Cardozo
 * Driver para RAK3172 en modo P2P
 * Comunicacion por UART (UCA0 - P1.4/P1.5)
 * Frecuencia: 915MHz (Argentina)
 */

#ifndef RAK_driver_H_
#define RAK_driver_H_

#include <stdint.h>
#include <stdbool.h>

void rak3172_init(void);
void rak3172_send(uint8_t status_byte);
void rak3172_wakeup(void);
void rak3172_sleep(void);

#endif /* RAK_driver_H_ */