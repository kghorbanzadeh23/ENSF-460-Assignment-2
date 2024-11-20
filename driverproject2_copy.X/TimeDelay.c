/* 
 * File:   TimeDelay.c
 * Author: Spiro, Kamand, Hutton
 *
 * Created on September 16, 2024, 3:58 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "TimeDelay.h"
uint8_t Delay_Flag = 0;

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
    
    PR2 = (time_ms * 62.5);// Calculate count value for timer
    
    TMR2 = 0;   //Set the timer2 flag to 1
    Delay_Flag = 0;
    
    T2CONbits.TON = 1;          // Enable timer 2
}

// Timer 2 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    Delay_Flag = 1;
    IFS0bits.T2IF = 0;  //Clear Flag
    T2CONbits.TON = 0;  //Disable Timer2
}