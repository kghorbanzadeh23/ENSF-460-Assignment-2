/* 
 * File:   IOs.c
 * Author: Spiro Douvis, Kamand Ghorbanzadeh, Hutton Ledingham
 *
 * Created on September 16, 2024, 3:55 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "IOs.h"
#include "TimeDelay.h"

//Define input and outputs for easier readability
#define PB1 PORTAbits.RA2 
#define PB2 PORTBbits.RB4
#define PB3 PORTAbits.RA4
#define LEDOUT LATBbits.LATB8

void IOinit(){
    
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    //Set pin B8 as an output pin
    TRISBbits.TRISB8 = 0;
    
    //Set A2, A4, and B4 inputs
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA4 = 1;
    TRISBbits.TRISB4 = 1;
    
    //Set pull-ups on the input pins
    CNPU2bits.CN30PUE = 1;
    CNPU1bits.CN1PUE = 1;
    CNPU1bits.CN0PUE = 1;
          
    //Set clock to 500 kHz
    newClk(32);
}

void IOcheck(){
     //Checks if more than one button is pushed, if so keep light on
        if(PB1 == 0 && PB2 == 0){
            //Set to the opposite state of the led status so that it would turn on and off causing the blink
            LEDOUT = !LEDOUT;
            
            delay_ms(1);
        } 
        //Check if first button (RA2) is pushed
        else if(PB1 == 0){
            //Set to the opposite state of the led status so that it would turn on and off causing the blink
            LEDOUT = !LEDOUT;
 
            //Stall processor cycles by going into a for loop to create a delay of about half a second
            delay_ms(500);
        } 
        //Check if button 2 (RB4) is pushed
        else if(PB2 == 0){
            //Set to the opposite state of the led status so that it would turn on and off causing the blink
            LEDOUT = !LEDOUT;
 
            //Stall processor cycles by going into a for loop to create a delay of about a second
            delay_ms(1000);

        } 
        //Check if button 3 (RA4) is pushed
        else if(PB3 == 0){
            //Set to the opposite state of the led status so that it would turn on and off causing the blink
            LEDOUT = !LEDOUT;
 
            //Stall processor cycles by going into a for loop to create a delay of about four seconds
            delay_ms(4000);

        } 
        //Only if no buttons are pushed
        else {
            //If no buttons are pushed turn off LED
            LEDOUT = 0;
            
            //Set clock lower to just for button checking.
            newClk(32);

        }
}