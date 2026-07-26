/*
 * sdc_drv.c
 *
 *  Created on: 28 mar. 2024
 *      Author: Salta
 */
#include <sdc_drv.h>
#include <msp430.h>

//variables de uso global
FATFS sdVolume;     // FatFs work area needed for each volume
FIL logfile;        // File object needed for each open file
char filename[] = "Test_N0.csv";
FILINFO fno;
FRESULT fr;



#pragma PERSISTENT(Number_File)

unsigned long Number_File = 48; //48= 0 en char
//TODO , 0 -9 , solo se puede apagar 10 veces el micro ....


/*
 * brief: Para chequear que este colocada la tarjeta sd . Por el momento medio rustico .
*/
DRESULT sdcardWaitCardInsert(BYTE drv){
    return (disk_initialize(drv)); /* Physical drive nmuber (0) */

}
void sd_file_check(){
    int number = filename[6];
    if (number==Number_File){
        SYSCFG0 = FRWPPW; // Program FRAM write enable
        Number_File++;
        SYSCFG0 = FRWPPW | PFWP; // Program FRAM write protecte
        filename[6]=Number_File;
    }

    return;



}
int sd_init(void){
    volatile int status;
  int kStatus_Success=sdcardWaitCardInsert(0);
    if (kStatus_Success!= 0)
        {
        //No  hay tarjeta.

        status=-1;
      }
    else {
    // Mount the SD Card
        switch( f_mount(&sdVolume, "", 0) ){
               case FR_OK:
                   status = 0;
                   break;
               case FR_INVALID_DRIVE:
                   status = 1;
                   break;
               case FR_DISK_ERR:
                   status = 2;
                   break;
               case FR_NOT_READY:
                   status = 3;
                   break;
               case FR_NO_FILESYSTEM:
                   status = 4;
                   break;
               default:
                   status = 5;
                   break;
               }
    }
    return status;
}

int sd_write(int d1,int d2, int d3, int d4, int d5,int d6, int d7, int d8){
    volatile int status;
    // Open & write
     if(f_open(&logfile, filename,  FA_OPEN_APPEND | FA_WRITE ) == FR_OK) {    // Open file - If nonexistent, create



          f_printf(&logfile,"%d;%d;%d;%d;%d;%d;%d;%d\n", d1,d2,d3,d4,d5,d6,d7,d8);
         // fr = f_write(&logfile, buffer,6, &bw);           /* Write it to the destination file */



          if(f_sync(&logfile)!=FR_OK){                          // Close the file
              status=-1;
          }
          else{
              status=0;
          }
        }
     else{
         status=1;
     }
     return status;
}


void sd_close(void){
    f_close(&logfile);
}


