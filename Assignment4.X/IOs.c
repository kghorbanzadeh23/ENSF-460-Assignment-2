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
    char clear[61] = "\033[2J \033[H "; //Clears terminal
    strcat(clear, message);             //Add on message after clear
    Disp2String(clear);                 //Display whole message
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    if(state == MODEZERO && !PB1){  //Check what state and PB1 is pressed
        state = MODEONE;            //Change the state to ModeOne if ModeZero and PB1
    }
    else if (state == MODEONE && !PB1){ //Check what state and PB1 is pressed
        state = MODEZERO;           //Change the state to ModeOne if ModeZero and PB1
        previousADCvalue = 1;       //Change to 1 so the screen will update
    }
    IFS1bits.CNIF = 0;     //Clear the CN interrupt flag
}

void IOcheck(){
    switch(state){
        case MODEZERO:;
            char line[45] = "Mode 0: "; //Starting of message
            LEDOUT = 0;                 //Turn LED off to help identify what mode it is in
            ADCvalue = do_ADC();        //Grab ADC value from pin 9
            uint8_t counter = ADCvalue * 0.031 + 1; //Calculate how many asterisks there are
            for(int i = 0; i < counter; i++){   //Add asterisks to message according the counter value is
                strcat(line, "*");  //Add asterisk to message
            }
            if(ADCvalue != previousADCvalue){   //If the value has changed since last time we check update the value
                previousADCvalue = ADCvalue;    //Set previous to current
                sendMessage(line);              //Send message over UART
                Disp2Hex(ADCvalue);             //Send ADC value in hex format
            }
            delay_ms(1000);                     //Delay for a second

            break;
        case MODEONE:
            LEDOUT = 1;     //Turn LED on to help debug what state
            char stringADC[10];     //Initialize array for UART message
            ADCvalue = do_ADC();    //Grab ADC value

            sprintf(stringADC, "%d \n", ADCvalue);  //Put ADC value in a string with correct formatting
            Disp2String(stringADC);                 //Send string over UART
            break;
    }
}