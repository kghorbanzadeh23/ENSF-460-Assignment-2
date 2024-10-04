/* 
 * File:   IOs.c
 * Author: Spiro, Kamand, Hutton
 *
 * Created on September 16, 2024, 3:55 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "IOs.h"
#include "TimeDelay.h"
#include "clkChange.h"

#define PB1 PORTAbits.RA2 
#define PB2 PORTBbits.RB4
#define PB3 PORTAbits.RA4
#define LEDOUT LATBbits.LATB8


typedef enum{
    NOTHING_PRESSED,
    BUTTON_PRESSED,
    BLINKING,
    LED_ON
} states;

states state = NOTHING_PRESSED;
uint16_t Blinking_Interval = 0;

uint8_t CNflag;

void IOinit(){
    
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    newClk(500);
    
    //T3CON config
    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
    T3CONbits.TCKPS = 1; // set prescaler to 1:8
    T3CONbits.TCS = 0; // use internal clock
    T3CONbits.TSIDL = 0; //operate in idle mode
    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; //enable timer interrupt
    PR3 = 15625/5; // set the count value for 0.5 s (or 500 ms)
    TMR3 = 0;
    T3CONbits.TON = 0;

    /* Let's set up some I/O */
    TRISBbits.TRISB8 = 0;
    LATBbits.LATB8 = 1;
    
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;
    
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;
    
    TRISAbits.TRISA2 = 1;
    CNPU2bits.CN30PUE = 1;
    CNEN2bits.CN30IE = 1;
    

    //Enable Interrupt
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;  //Clear Flag
    IEC1bits.CNIE = 1;  
    
    //Default state
    state = NOTHING_PRESSED;
    Blinking_Interval = 0;
    CNflag = 0;
}

void IOcheck(){
    switch(state){
        case NOTHING_PRESSED:
            sendMessage("Nothing pressed\n\r");
            LEDOUT = 0;

            Idle();
            break;
            
        case BUTTON_PRESSED:
            CNflag = 0;
            T3CONbits.TON = 1;  //Turn timer on to prevent debounce
            Idle();
            
            if(CNflag){
                break;  //If debounce occurs or new button is pressed leave this case
            }
            
            if (PB1 && PB2 && PB3){
                state = NOTHING_PRESSED;

            }
            else if (!PB1 && !PB2 && !PB3){
                sendMessage("All buttons pressed\n\r");
                state = LED_ON;
            }
            else if (!PB2 && !PB1) {
                sendMessage("PB1 and PB2 pressed\n\r");
                state = LED_ON;
            }
            else if (!PB3 && !PB2) {
                sendMessage("PB2 and PB3 pressed\n\r");
                state = LED_ON;
            }
            else if (!PB3 && !PB1) {
                sendMessage("PB1 and PB3 pressed\n\r");
                state = LED_ON;
            }
            else if (!PB1){
                sendMessage("PB1 event\n\r");
                state = BLINKING;
                Blinking_Interval = 500;
            }
            else if(!PB2){
                sendMessage("PB2 event\n\r");
                state = BLINKING;
                Blinking_Interval = 1000;
            }
            else if(!PB3){
                sendMessage("PB3 event\n\r");
                state = BLINKING;
                Blinking_Interval = 4000;
            }

            break;
            
        case BLINKING:
            LEDOUT = !LEDOUT;
            delay_ms(Blinking_Interval);
            break;
        case LED_ON:
            LEDOUT = 1;
            Idle();
            break;
    }
}
    
void sendMessage(char* message){
    //Clear terminal
    Disp2String("\033[2J");
    //Makes cursor goto top left
    Disp2String("\033[H");
    //Send message
    Disp2String(message);
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
         //Don't forget to clear the CN interrupt flag!
    CNflag = 1;
    state = BUTTON_PRESSED;
    T2CONbits.TON = 0;
    T3CONbits.TON = 0;

    IFS1bits.CNIF = 0;
}

// Timer 2 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    IFS0bits.T2IF = 0;  //Clear Flag
    T2CONbits.TON = 0;  //Disable Timer2

}

// Timer 3 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!
    IFS0bits.T3IF = 0;  //Clear flag
    T3CONbits.TON = 0;  //Disable Timer3

    TMR3 = 0;
}

