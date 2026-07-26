/*
 * sdc_drv.h
 *
 *  Created on: 28 mar. 2024
 *      Author: Salta
 */

#ifndef SDC_DRV_H_
#define SDC_DRV_H_
#include <diskio.h>             //use mismo codigos de error.
#include <ff.h>

DRESULT sdcardWaitCardInsert(BYTE drv);             //Chequeamos que la sd este fisicamente puesta.
int sd_init(void);                              // es cero, entonces inicializamos bien.

int sd_write(int d1,int d2, int d3, int d4, int d5,int d6, int d7, int d8);        //devuelve cero si esta bine.
void sd_close(void);
void sd_file_check();
#endif /* SDC_DRV_H_ */
