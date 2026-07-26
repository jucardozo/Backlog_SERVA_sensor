/*
 * uart_drv.h
 *
 *  Created on: 17 ene. 2024
 *      Author: joaco
 */

#ifndef JOACO_UART_DRV_UART_DRV_H_
#define JOACO_UART_DRV_UART_DRV_H_

void init_uart_drv(); // inicializa el driver de la UART
void send_msg(char* message, int message_len); // para mandar una palabra de chars

//la dinámica de recepcion es la siguiente: chequeas si el buffer de recepción está
//vacío con rx_buffer_empty() (devuelve un bool) y si no está vacío levantas el proximo char con get_last_rx_byte()
char get_last_rx_byte();
char rx_buffer_empty();

#endif /* JOACO_UART_DRV_UART_DRV_H_ */
