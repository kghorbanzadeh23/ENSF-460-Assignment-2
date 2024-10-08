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

//Define input and outputs for easier readability
#define PB1 PORTAbits.RA2 
#define PB2 PORTBbits.RB4
#define PB3 PORTAbits.RA4
#define LEDOUT LATBbits.LATB8


//Different states
typedef enum{
    NOTHING_PRESSED,    //When no buttons are pressed
    BUTTON_PRESSED,     //When a button is pushed or let go
    BLINKING,           //If a button is pushed and cause blinking
    LED_ON              //Keeps LED_ON if more than 2 buttons are on
} states;

states state = NOTHING_PRESSED; //Keeps track of state
uint16_t Blinking_Interval = 0; //Tracks the Blinking_interval

uint8_t CNflag; //Tracks if there was a CN interrupt

void IOinit(){
    
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    newClk(500);    //Switch clock to 500khz
    
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
    TRISBbits.TRISB8 = 0;   //Set as output
    
    //Set as input
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;   //Enable pull-up
    CNEN1bits.CN0IE = 1;    //Enable CN interrupts
    
    TRISBbits.TRISB4 = 1;   //Set as input
    CNPU1bits.CN1PUE = 1;   //Enable pull-up
    CNEN1bits.CN1IE = 1;    //Enable CN interrupt
    
    TRISAbits.TRISA2 = 1;   //Set as input
    CNPU2bits.CN30PUE = 1;  //Enable pull-up
    CNEN2bits.CN30IE = 1;   //Enable CN interrupt
    

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
        case NOTHING_PRESSED:        //State for when no buttons are pressed
            sendMessage("Nothing pressed\n\r"); //Display message that no buttons are pressed
            LEDOUT = 0;             //Turn led out

            Idle();                 //Idle until next interrupt either the timer or a new button pressed
            break;
            
        case BUTTON_PRESSED:        //When any of the buttons states changed
            CNflag = 0;             //Reset CNflag
            T3CONbits.TON = 1;  //Turn timer on to prevent debounce
            Idle();             //Stay here until the timer interrupt or button states change
            
            if(CNflag){
                break;  //If debounce occurs or new button is pressed leave this case and read the button states again
            }
            
            //Which push buttons are pressed and which combination is pressed
            if (PB1 && PB2 && PB3){     //If all buttons are not pressed
                state = NOTHING_PRESSED;    //Changed state 

            }
            else if (!PB1 && !PB2 && !PB3){ //If all buttons are pressed 
                sendMessage("All buttons pressed\n\r"); //Display message according to combination of buttons that are pressed
                state = LED_ON;                         //Switch state to LED_ON
            }
            else if (!PB2 && !PB1) { //If Push button 1 and 2 are pressed
                sendMessage("PB1 and PB2 pressed\n\r"); //Display message according to combination of buttons that are pressed
                state = LED_ON;
            }
            else if (!PB3 && !PB2) { //If push buttons 3 and 2 are pressed
                sendMessage("PB2 and PB3 pressed\n\r"); //Display message according to combination of buttons that are pressed
                state = LED_ON;
            }
            else if (!PB3 && !PB1) { //If push buttons 3 and 1 are pressed
                sendMessage("PB1 and PB3 pressed\n\r"); //Display message according to combination of buttons that are pressed
                state = LED_ON;
            }
            else if (!PB1){ //If only push button 1 is pressed
                sendMessage("PB1 event\n\r"); //Display message according to which push button is pressed
                state = BLINKING;             //Switch state to blinking as only one button is pressed
                Blinking_Interval = 500;      //Set the blinking interval to 500ms
            }
            else if(!PB2){ //If only push button 2 is pressed
                sendMessage("PB2 event\n\r"); //Display message according to which push button is pressed
                state = BLINKING;
                Blinking_Interval = 1000;   //Set the blinking interval to 1000ms
            }
            else if(!PB3){ //If only push button 3 is pressed
                sendMessage("PB3 event\n\r"); //Display message according to which push button is pressed
                state = BLINKING;
                Blinking_Interval = 4000;   //Set the blinking interval to 4000ms
            }

            break;
            
        case BLINKING:  //Code to handle the blinking
            LEDOUT = !LEDOUT;   //Switch light to opposite state 
            delay_ms(Blinking_Interval);    //Pause the code here until an interrupt 
            break;
        case LED_ON:    //Code to handle the multiple button pushes
            LEDOUT = 1; //Turn LED on
            Idle();     //Wait here until button states change
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
    CNflag = 1;     //Flag to detect a CN interrupt
    state = BUTTON_PRESSED; //Change state to BUTTON_PRESSED as the states of the button changed
    T2CONbits.TON = 0;      //Disable timer 2
    T3CONbits.TON = 0;      //Disable timer 3

    IFS1bits.CNIF = 0;     //Clear the CN interrupt flag
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

}

