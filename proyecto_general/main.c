#include <msp430.h> 
#include <Board.h>
#include <driverlib/gpio.h>
#include <driverlib/pmm.h>
#include <stdbool.h>
#include <stdio.h>
#include "low_power_manager.h"
#include "auxfuncs.h"
#include "lpfilter.h"
#include "sensor_board.h"
#include "sdc_drv.h"
#include "uart_drv.h"
#include <string.h>

/***************************/
/* FUNCTION DECLARATIONS   */
/***************************/

void startup_sequence();
bool collect_sample(int16_t*readingval);
bool is_rf_time();
void int16_to_string(char* result, int16_t number);
void send_rf_signal();
void send_rf_msg_countdown(int countdown);
void initClockTo8MHz();
void wakeup_rf();
void send2sleep_rf();

/******************/
/*    DEFINES     */
/******************/

#define BUFFERLEN 65
#define BUFFERLENRAW 128
#define ENVELOPEBUFFERLEN 100
#define THRESHHDOLD 45
#define COUNT_TOTAL 30
#define REAL_MINUTES_TILL_RF 1
#define CYCLES_TILL_RF REAL_MINUTES_TILL_RF*(60/COUNT_TOTAL)


/***************************/
/*   GLOBAL VARIABLES      */
/***************************/

//Buffers:

int16_t last_mag_readings [BUFFERLENRAW] = {0};
int16_t last_mag_readings_meanzero [BUFFERLEN] = {0};
int16_t last_mag_readings_envelope [ENVELOPEBUFFERLEN] = {0};

//Variables para guardar info en la SD:

int16_t acc_SD = 0;

int16_t accX_SD = 0;

int16_t accY_SD = 0;

int16_t accZ_SD = 0;


int16_t mag_SD = 0;

int16_t magX_SD = 0;

int16_t magY_SD = 0;

int16_t magZ_SD = 0;

int16_t output_prethershhold = 0;
int16_t isitturning_SD = 0;


/*******************/
/*    MAIN         */
/*******************/


int main(void)
      {
    WDTCTL = WDTPW | WDTHOLD;                                     // Stop watchdog timer
    initClockTo8MHz(); // iniciamos el clock del proce en 8MHz, esto hay que tener cuidado con tocarlo porque afecta los drivers de UART e i2c

    //Inicializamos la comunicaci n con sensores
    init_magacc_driver();

    //Set LED1 to output direction
    GPIO_setAsOutputPin(
        GPIO_PORT_P3,
        GPIO_PIN6
        );

    wakeup_rf();




    char buffer [] = "-Program Start\n\0";
    send_msg(buffer, strlen(buffer)); //Mandamos por RF mensaje de inicializacion del programa

    //Hacemos el proceso de inicializacion del driver de la SD
    _delay_cycles(15000);

    int status = sd_init();

    _delay_cycles(15000);

    void sd_file_check();

    _delay_cycles(15000);

    //Si no se trab  hasta ac  es porque no cay  en ningun trap, inicializ  SD bien
    //Mandamos mensaje
    if(status == 0){
        char buffer2 [] = "-Start SD OK\n\0";
        send_msg(buffer2, strlen(buffer2));
    }
    else{

        char buffer23 [] = "-Start SD ERROR!";
        send_msg(buffer23, strlen(buffer23));

        char buffer20 [] = "-ERROR: ";
        send_msg(buffer20, strlen(buffer20));

        char buffer21[7];
        int16_to_string(buffer21, (int16_t)status);
        send_msg(buffer21, strlen(buffer21));

        char buffer22 [] = "\n\0";
        send_msg(buffer22, strlen(buffer22));

    }



    //Vemos si el aceler metro detecta bien
    _delay_cycles(100000);

    int16_t a,b,c = 0;
    read_acc(&a, &b, &c);

    int16_t acc_read = a;

    if(acc_read==0){
        char buffer4 [] = "-Error Acc Read\n\0";
        send_msg(buffer4, strlen(buffer4));
        _delay_cycles(5000); //Si no anda el aceler metro fue
    }
    else{
        char buffer4 [] = "-Acc Read OK\n\0";
        send_msg(buffer4, strlen(buffer4));
    }

    //Vemos si el magnet metro detecta bien

    _delay_cycles(100000);
    read_mag(&a, &b, &c);

    int16_t mag_read = a;

    if(mag_read==0){
        char buffer3 [] = "-Error Mag Read\n\0";
        send_msg(buffer3, strlen(buffer3));
    }
    else{
        char buffer3 [] = "-Mag Read OK\n\0";
        send_msg(buffer3, strlen(buffer3));
    }

    send2sleep_rf();




    init_timer(800); //inicializamos el RTC para que se despierte cada 1s
                     //deber a ser 1000 el numero pero no se por que le pinto que sea 745


    //Se inicializa un periodo de warm up para que se instale el aparato antes de que empiece
    //el algoritmo a laburar. Solo transmite mediciones por RF como keep alive.

    #define SEC_COUNT 5
    int second_count = SEC_COUNT; //Transmite cada 15s
    int minutes_to_begin = 1; //IMPORTANTE: CANTIDAD DE MINUTOS HASTA QUE SALGA DEL LOOP Y EMPIECE EL ALGORITMO DE DETECCION
    int cycles_to_begin = minutes_to_begin*60/second_count;



    while((cycles_to_begin--)>0){

        wakeup_rf();

        reinit_magacc_driver();

        send_rf_msg_countdown(cycles_to_begin);

        send2sleep_rf();

        while((second_count--)>0){
            enter_lpm();
        }
        second_count = SEC_COUNT;  //Si decidimos cambiar la cant. de segundos, cambiar esta linea tmb

        _delay_cycles(10000);

    }



    //Precargamos el buffer de entrada con la primera medici n, estimando que la media muy lejos no va a estar.


    reinit_magacc_driver();
    _delay_cycles(5000);
    int16_t reading, _, __ = 0;
    read_mag(&reading, &_, &__);

    int i = 0;
    for(i=0; i<BUFFERLENRAW; i++){
        last_mag_readings[i] = reading;
    }

    wakeup_rf();
    char buffer18 [] = "\n\0\n\0/Begin Operation:\n\0\n\0";
    send_msg(buffer18, strlen(buffer18));

    send2sleep_rf();

    // Ac  comienza el main loop del programa

    while(1)
    {

        reinit_magacc_driver();

        //Primero tomamos la medici n del magnet metro y la guardamos la version cruda en la tarjeta SD

        _delay_cycles(5000);

        int16_t accval_x = 0;
        int16_t accval_y = 0;
        int16_t accval_z = 0;

        read_acc(&accval_x, &accval_y, &accval_z);

        _delay_cycles(5000);

        acc_SD = accval_x;

        accX_SD = accval_x;
        accY_SD = accval_y;
        accZ_SD = accval_z;



        _delay_cycles(20000);

        int16_t readingval = 0;

        //collect_sample va tomando muestras y cuando llega a un numero predeterminado almacena el promedio
        //en readingval y devuelve true

        if(collect_sample(&readingval))
        {



        append_and_shift(last_mag_readings, BUFFERLENRAW, readingval);

        int16_t mean_corrected_readingval =  readingval-get_mean(last_mag_readings,BUFFERLENRAW);


        //esta es la parte del filtrado LP que sacamos xq los pozos pueden girar bastante m s r pido de lo que pensabamos

                    /*
                    append_and_shift(last_mag_readings_meanzero, BUFFERLEN, mean_corrected_readingval);


                    int16_t lpfiltered_readingval = apply_filter(last_mag_readings_meanzero);

                    if(lpfiltered_readingval<0){
                        lpfiltered_readingval=-lpfiltered_readingval;
                    }
                    append_and_shift(last_mag_readings_envelope, ENVELOPEBUFFERLEN, lpfiltered_readingval);

                    */


        //Ahora rectificamos el valor del arreglo con media 0 y lo guardamos en last_mag_readings_envelope

        if(mean_corrected_readingval<0){
            mean_corrected_readingval=-mean_corrected_readingval;
        }

        append_and_shift(last_mag_readings_envelope, ENVELOPEBUFFERLEN, mean_corrected_readingval);

        //Cuando llegamos a la cantidad de minutos necesarios para mandar los datos por RF, los mandamos
        if(is_rf_time()){
            int16_t result = get_mean(last_mag_readings_envelope,ENVELOPEBUFFERLEN);
            __no_operation();


            bool isitturning = false;

            isitturning = (result > THRESHHDOLD);

            isitturning_SD = (int16_t) isitturning;
            output_prethershhold = (int16_t) result;

            wakeup_rf();

            send_rf_signal();

            _delay_cycles(200000);

            send2sleep_rf();
        }


        }

        static int count = 0;

        sd_write((int)accX_SD, (int)accY_SD, (int)accZ_SD, (int)magX_SD, (int)magY_SD, (int)magZ_SD, (int)output_prethershhold, count);

        count++;
        if(count==32000){
            count = 0;
        }


        _delay_cycles(10000);
        enter_lpm();
    }
}

/*******************/
/* HELPER FUNCTIONS */
/*******************/

void int16_to_string(char* result, int16_t number) {
    // Handle the sign and assign the first character accordingly
    if (number < 0) {
        result[0] = '-';
        number = -number;  // Make the number positive for further processing
    } else {
        result[0] = '+';
    }

    // Convert the number to string and store it starting from the second character
    snprintf(result + 1, 6, "%d", number);
    return;
}

bool collect_sample(int16_t*readingval){

    /* Esta funcion llama a get_mag_reading 60 veces, cuando cumplio con este numero
     * devuelve true y en el puntero parametro carga el valor del promedio de las mediciones
     */

    static int16_t count = 0;
    static int32_t suma = 0;

    int16_t magval_x = 0;
    int16_t magval_y = 0;
    int16_t magval_z = 0;

    read_mag(&magval_x, &magval_y, &magval_z);

    suma = suma + (int32_t)magval_z;
    count += 1;

    _delay_cycles(5000);

    mag_SD = magval_x;

    magX_SD = magval_x;
    magY_SD = magval_y;
    magZ_SD = magval_z;

    bool retval = false;
    if(count == COUNT_TOTAL){
        count = 0;
        int32_t tentative_readingval = suma/COUNT_TOTAL;
        suma = 0;
        int16_t aux = (int16_t) tentative_readingval;
        *readingval = aux;
        retval = true;
    }

    return retval;
}

bool is_rf_time() {
    static int count = 0;

    count+=1;
    bool ret = false;
    if(count == CYCLES_TILL_RF){
        ret=true;
        count = 0;
    }
    return ret;
}

void send_rf_signal(){
   char buffer9 [] = "/";
   send_msg(buffer9, strlen(buffer9));


   char buffer10 [] = "acc: ";
   send_msg(buffer10, strlen(buffer10));


   char buffer[7];

   int16_to_string(buffer,(int16_t)accX_SD);
   send_msg(buffer, strlen(buffer));

   char buffer11 [] = "/mag: ";
   send_msg(buffer11, strlen(buffer11));

   int16_to_string(buffer, (int16_t)magY_SD);
   send_msg(buffer, strlen(buffer));


   char buffer2[5];
   buffer2[0] = '/';
   buffer2[1] = '0';
   buffer2[2] = isitturning_SD+'0';
   buffer2[3] = '\n';
   buffer2[4] = '\0';
   send_msg(buffer2, strlen(buffer2));
   return;
}


void initClockTo8MHz()
{
    __bis_SR_register(SCG0);                 // disable FLL
     CSCTL3 |= SELREF__REFOCLK;               // Set REFO as FLL reference source
     CSCTL0 = 0;                              // clear DCO and MOD registers
     CSCTL1 &= ~(DCORSEL_7);                  // Clear DCO frequency select bits first
     CSCTL1 |= DCORSEL_3;                     // Set DCO = 8MHz
     CSCTL2 = FLLD_0 + 243;                   // DCODIV = 8MHz
     __delay_cycles(3);
     __bic_SR_register(SCG0);                 // enable FLL
     while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked

     CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                              // default DCODIV as MCLK and SMCLK source
}

void send_rf_msg_countdown(int cycles_to_begin){

            _delay_cycles(20000);
            int16_t a,b,c;
            read_acc(&a,&b,&c);
            int16_t acc = a;

            _delay_cycles(10000);
            read_mag(&a,&b,&c);
            int16_t reading = b;


            _delay_cycles(5000);

            char buffer4 [] = "/";
            send_msg(buffer4, strlen(buffer4));

            char buffer11 [] = "acc: ";
            send_msg(buffer11, strlen(buffer11));

            char buffer[7];
            int16_to_string(buffer, (int16_t)acc);
            send_msg(buffer, strlen(buffer));

            send_msg(buffer4, strlen(buffer4));

            char buffer10 [] = "mag: ";
            send_msg(buffer10, strlen(buffer10));

            int16_to_string(buffer,(int16_t)reading);
            send_msg(buffer, strlen(buffer));

            send_msg(buffer4, strlen(buffer4));

            char buffer5 [] = "/count:";
            send_msg(buffer5, strlen(buffer5));

            int16_to_string(buffer, cycles_to_begin);
            send_msg(buffer, strlen(buffer));


            send_msg(buffer4, strlen(buffer4));

            buffer4[0] = '\n';
            send_msg(buffer4, 1);
            buffer4[0] = '\0';
            send_msg(buffer4, 1);
}

void wakeup_rf(){

    GPIO_setOutputLowOnPin(
                GPIO_PORT_P3,
                GPIO_PIN6
                );

        _delay_cycles(200000);
        return;
}

void send2sleep_rf(){


    _delay_cycles(600000);

    GPIO_setOutputHighOnPin(
            GPIO_PORT_P3,
            GPIO_PIN6
            );

    return;

}








