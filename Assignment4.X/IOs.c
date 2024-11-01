/*
 * File:   IOs.c
 * Author: Spiro, Kamand, Hutton
 *
 * Created on October 31, 2024, 10:25 AM
 */


#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include "IOs.h"
#include "TimeDelay.h"
#include "clkChange.h"
#include "UART.h"
#include "ADC.h"


#define PB1 PORTAbits.RA2 
#define LEDOUT LATBbits.LATB8

typedef enum{
    MODEZERO,    //When no buttons are pressed
    MODEONE//When the timer reaches to 0
} states;
states state; //Keeps track of state
uint16_t ADCvalue;
uint16_t previousADCvalue = 1;
void IOinit(){
    TRISAbits.TRISA2 = 1;   //Set as input
    CNPU2bits.CN30PUE = 1;  //Enable pull-up
    CNEN2bits.CN30IE = 1;   //Enable CN interrupt
    
    //Enable Interrupt
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;  //Clear Flag
    IEC1bits.CNIE = 1;  
    
    TRISBbits.TRISB8 = 0;   //Set as output

}

void StateInit(){
    state = MODEZERO;            
    
}

void sendMessage(char* message){
    char clear[61] = "\033[2J \033[H ";
    strcat(clear, message);
    Disp2String(clear);
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    if(state == MODEZERO && !PB1){
        state = MODEONE;
    }
    else if (state == MODEONE && !PB1){
        state = MODEZERO;
        previousADCvalue = 1;
    }
    IFS1bits.CNIF = 0;     //Clear the CN interrupt flag
}

void IOcheck(){
    switch(state){
        case MODEZERO:;
            char line[45] = "Mode 0: ";
            LEDOUT = 0;
            ADCvalue = do_ADC();
            uint8_t counter = ADCvalue * 0.031 + 1;
            for(int i = 0; i < counter; i++){
                strcat(line, "*");
            }
            if(ADCvalue != previousADCvalue){
                previousADCvalue = ADCvalue;
                sendMessage(line);
                Disp2Hex(ADCvalue);
            }
            delay_ms(1000);

            break;
        case MODEONE:
            LEDOUT = 1;
            char stringADC[10];
            ADCvalue = do_ADC();

            sprintf(stringADC, "%d \n", ADCvalue);
            Disp2String(stringADC);
            break;
    }
}