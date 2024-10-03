/* 
 * File:   TimeDelay2.c
 * Author: spiro
 *
 * Created on September 16, 2024, 3:58 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "TimeDelay2.h"


void delay_ms(uint16_t time_ms){
    newClk(500);
    
    if(time_ms > 16778){
        time_ms = 16778;
    }
    
    //Timer2 config
    T2CONbits.T32 = 0;      // operate timer 2 as 16 bit timer
    T2CONbits.TCKPS = 2;    // set pre-scaler
    T2CONbits.TCS = 0;      // use internal clock
    T2CONbits.TSIDL = 0;    // operate in idle mode
    
    //Timer 2 interrupt config
    IPC1bits.T2IP = 2;      // Set priority for interrupt
    IFS0bits.T2IF = 0;      // clear interrupt flag
    IEC0bits.T2IE = 1;      // enable timer interrupt
    
    
    if(time_ms <= 200){
        T2CONbits.TCKPS = 0;    // set pre-scaler for 1ms
        PR2 = time_ms * 250;  // Calculate count for 1ms
    }   
    else{
        PR2 = (time_ms * 3.906);// Calculate count value for timer
    }
    TMR2 = 1;

    T2CONbits.TON = 1;          // Enable timer 2
    Idle();    //Run until timer interrupt
}

