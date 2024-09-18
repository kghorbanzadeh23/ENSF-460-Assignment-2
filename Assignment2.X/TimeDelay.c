/* 
 * File:   TimeDelay.c
 * Author: Spiro Douvis, Kamand Ghorbanzadeh, Hutton Ledingham
 *
 * Created on September 16, 2024, 3:58 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "TimeDelay.h"


void delay_ms(uint16_t time_ms){
    
    //Timer2 config
    T2CONbits.T32 = 0;      // operate timer 2 as 16 bit timer
    T2CONbits.TCKPS = 2;    // set pre-scaler
    T2CONbits.TCS = 0;      // use internal clock
    T2CONbits.TSIDL = 0;    // operate in idle mode
    
    //Timer 2 interrupt config
    IPC1bits.T2IP = 2;      // Set priority for interrupt
    IFS0bits.T2IF = 0;      // clear interrupt flag
    IEC0bits.T2IE = 1;      // enable timer interrupt
    
    PR2 = (time_ms * 3.906);// Calculate count value for timer
    
    T2CONbits.TON = 1;      // Enable timer 2

    //Run until timer interrupt
    Idle();
}

