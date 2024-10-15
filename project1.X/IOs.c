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
    TIMER_CHANGE,
    TIMER_COUNTDOWN,              
    TIMER_PAUSED,
    TIMER_IDLE,
    TIMER_COMPLETED
} states;

states state = NOTHING_PRESSED; //Keeps track of state
uint16_t Blinking_Interval = 0; //Tracks the Blinking_interval
uint8_t seconds = 0;
uint8_t minutes = 0;
int8_t deltaSec = 0;
int8_t deltaMin = 0;
uint8_t CNflag; //Tracks if there was a CN interrupt
uint8_t timerPaused = 1;
uint8_t timer3Flag = 0;
uint8_t timerActive = 0;
uint8_t PB2Counter = 0;

void IOinit(){
    
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    newClk(500);    //Switch clock to 500khz
    
//    //T3CON config
//    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
//    T3CONbits.TCKPS = 1; // set prescaler to 1:8
//    T3CONbits.TCS = 0; // use internal clock
//    T3CONbits.TSIDL = 0; //operate in idle mode
//    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
//    IFS0bits.T3IF = 0;
//    IEC0bits.T3IE = 1; //enable timer interrupt
//    PR3 = 15625/5; // set the count value for 0.5 s (or 500 ms)
//    TMR3 = 0;
//    T3CONbits.TON = 0;

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

            Idle();                 //Idle until next interrupt either the timer or a new button pressed
            break;
            
        case BUTTON_PRESSED:        //When any of the buttons states changed
            if(!PB3 && PB2 && PB3){
                T3CONbits.TON = 0;
                CNflag = 0;
                delay_ms(3000);
                T2CONbits.TON = 0;
                
                if(CNflag && seconds && minutes){
                    if(!timerActive){
                        startTimer();
                        timerPaused = 1;
                        break;
                    }
                    
                    if(timerPaused){
                       state = TIMER_IDLE;
                       timerPaused = 0;
                    }
                    else if(!timerPaused){
                        timerPaused = 1;
                        state = TIMER_PAUSED;
                    }

                }
                else if(!CNflag){
                    seconds = 0;
                    minutes = 0;
                    state = NOTHING_PRESSED;
                    sendMessage("CLR ");
                }
                else{
                    state = NOTHING_PRESSED;
                }
                break;
            }
            
            
            if(!timerPaused){
                break;
            }
            
            CNflag = 0;             //Reset CNflag
            delay_ms(50);
            
            if(CNflag){
                break;  //If debounce occurs or new button is pressed leave this case and read the button states again
            }
            
            //Which push buttons are pressed and which combination is pressed
            if (!PB2 && !PB1) { //If Push button 1 and 2 are pressed
                deltaMin = 1;
                deltaSec = 1;
                state = TIMER_CHANGE;             //Switch state to blinking as only one button is pressed
            }
            else if (!PB1){ //If only push button 1 is pressed
                deltaSec = 0;
                deltaMin = 1;
                state = TIMER_CHANGE;             //Switch state to blinking as only one button is pressed
            }
            else if(!PB2){ //If only push button 2 is pressed
                deltaSec = 1;
                deltaMin = 0;
                state = TIMER_CHANGE;             //Switch state to blinking as only one button is pressed
            }
            else{
                PB2Counter = 0;
                state = NOTHING_PRESSED;
            }


            break;
            
        case TIMER_CHANGE:
            T3CONbits.TON = 1;  
            changeTime(deltaSec, deltaMin);
            sendMessage("SET ");
            delay_ms(100);    //Pause the code here until an interrupt 
            
            if(PB2Counter >= 10){
                deltaSec = 5;
            }
            else{
                PB2Counter++;
            }
            break;
        case TIMER_COUNTDOWN:    //Code to handle the multiple button pushes
            
            LEDOUT = !LEDOUT;   //Switch light to opposite state 
            changeTime(-1,0);
            if(seconds == 0 && minutes == 0){
                state = TIMER_COMPLETED;
            }
            else{
                sendMessage("CNT ");
            }

            break;
        case TIMER_PAUSED:
            
            Idle();
            break;
        case TIMER_COMPLETED:
            timerActive = 0;
            T3CONbits.TON = 0;
            LEDOUT = 1;
            sendMessage("FIN ");
            Idle();
            
            break;
        case TIMER_IDLE:
            Idle();
            break;
    }
}
    

startTimer(){
    TMR3 = 0;
    T3CONbits.TON = 1;      //Start timer 3
}
void sendMessage(char* message){
    char strMIN[5];
    char strSEC[5];
    
    sprintf(strMIN, "%02dm", minutes);
    sprintf(strSEC, "%02ds", seconds);
    
    char fullMessage[50];
    
    strcpy(fullMessage, message);
    strcat(fullMessage, strMIN);
    strcat(fullMessage, " : ");
    strcat(fullMessage, strSEC);
    
    if(state == TIMER_COMPLETED){
        strcat(fullMessage, " -- ALARM");
    }
            
    //Clear terminal
    Disp2String("\033[2J");
    //Makes cursor goto top left
    Disp2String("\033[H");
    //Send message
    Disp2String(fullMessage);
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    CNflag = 1;     //Flag to detect a CN interrupt
    state = BUTTON_PRESSED; //Change state to BUTTON_PRESSED as the states of the button changed
    T3CONbits.TON = 0;      //Disable timer 3
    IFS1bits.CNIF = 0;     //Clear the CN interrupt flag
}

// Timer 3 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!
    IFS0bits.T3IF = 0;  //Clear flag
    T3CONbits.TON = 0;  //Disable Timer3
    timer3Flag = 1;
    TMR3 = 0;
    state = TIMER_COUNTDOWN;
}


void changeTime(int8_t changeSec, int8_t changeMinute){
    seconds += changeSec;
    minutes += changeMinute;
    
    if(seconds == 60){
        seconds = 0;
        minutes++;
    }
    
    if(minutes == 60){
        minutes = 0;
    }
    
}

startTimer3(){
    //T3CON config
    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
    T3CONbits.TCKPS = 2; // set prescaler to 1:8
    T3CONbits.TCS = 0; // use internal clock
    T3CONbits.TSIDL = 0; //operate in idle mode
    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; //enable timer interrupt
    PR3 = 3.906*1000; // set the count value for 0.5 s (or 500 ms)
    TMR3 = 0;
    T3CONbits.TON = 0;
    timerActive = 1;
}

