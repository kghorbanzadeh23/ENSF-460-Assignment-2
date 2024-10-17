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
    TIMER_CHANGE,       //When changing the time with PB1 and PB2
    TIMER_COUNTDOWN,    //To update the timer when timer3 interrupts
    TIMER_PAUSED,       //For when the timer is paused
    TIMER_IDLE,         //When the program is waiting for an interrupt
    TIMER_COMPLETED     //When the timer reaches to 0
} states;

states state = NOTHING_PRESSED; //Keeps track of state
uint16_t Blinking_Interval = 0; //Tracks the Blinking_interval
uint8_t seconds = 0;    //Keep track of seconds
uint8_t minutes = 0;    //Keep track of minutes
int8_t deltaSec = 0;    //To keep track of what to change the seconds by according to the buttons
int8_t deltaMin = 0;    //To keep track of what to change the minutes by according to the buttons
uint8_t CNflag; //Tracks if there was a CN interrupt
uint8_t timerPaused = 0;    //Keeps rtrack if timer is paused
uint8_t timerActive = 0;    //Keeps tracks if the timer is active 
uint8_t PB2Counter = 0;     //For the functionality of incrementing the seconds by 5

void IOinit(){
    
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    newClk(500);    //Switch clock to 500khz
    
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
        case NOTHING_PRESSED:        //State for when no buttons are pressed and timer is not active only when setting the timer
            LEDOUT = 0;             
            Idle();                 //Idle until next interrupt either the timer or a new button pressed
            break;
            
        case BUTTON_PRESSED:        //When any of the buttons states changed
            if(!PB3 && PB2 && PB1){ //Checks if only PB3 was pressed
                T3CONbits.TON = 0;  //Disable the countdown
                CNflag = 0;         //Set up for checking for PB3 > 3 seconds

                delay_ms(3000);     //delay_ms goes into idle so either the timer interrupt or button
                T2CONbits.TON = 0;
                
                if(CNflag && (seconds || minutes)){ //Checks if button caused interrupt
                    if(!timerActive){   //If the timer hasn't been started yet
                        startTimer();   //Starts the timer
                        timerActive = 1;    //Sets flag
                        sendMessage("CNT ");    //Send countdown message

                        break;
                    }
                    
                    if(timerPaused){    //If timer has been started check if paused
                       state = TIMER_IDLE;  //Sets to timer_idle if paused
                       T3CONbits.TON = 1;   //Re-enable the timer

                       timerPaused = 0;     //Disable the paused flag
                    }
                    else if(!timerPaused){  //If timer is not paused
                        timerPaused = 1;    //Set flag to paused
                        state = TIMER_PAUSED;   //State to TIMER_PAUSED
                    }

                }
                else if(!CNflag){   //If the timer caused the interrupt
                    seconds = 0;    //Reset the seconds
                    minutes = 0;    //Reset the minues
                    state = NOTHING_PRESSED;    //Go back to default state
                    timerPaused = 0;            //Reset to dafault states
                    timerActive = 0;
                    TMR3 = 0;                   //Reset TMR3 counter
                    
                    sendMessage("CLR ");        //Send clear message
                }
                else{
                    state = NOTHING_PRESSED;    //If seconds and minutes are not set go back to nothing pressed

                }
                break;
            }
            
            
            if(timerActive){    //If the timer is active ignore other inputs
                break;
            }
            
            CNflag = 0;             //Reset CNflag
            delay_ms(50);           //To prevent bounce on the buttons
            
            if(CNflag){
                break;  //If debounce occurs or new button is pressed leave this case and read the button states again
            }
            
            
            //Which push buttons are pressed and which combination is pressed
            if (!PB2 && !PB1) { //If Push button 1 and 2 are pressed
                deltaMin = 1;   //Changing the seconds by 1
                deltaSec = 1;   //Changing the minutes by 1
                state = TIMER_CHANGE;   //Switch state to TIMER_CHANGE to update the minutes and seconds
            }
            else if (!PB1){ //If only push button 1 is pressed
                deltaSec = 0;   //Changing the seconds by 0
                deltaMin = 1;   //Changing the minutes by 1
                state = TIMER_CHANGE;   //Switch state to TIMER_CHANGE to update the minutes and seconds           
                PB2Counter = 0; //Reset PB2 coutner as PB2 is not pressed
            }
            else if(!PB2){ //If only push button 2 is pressed
                deltaSec = 1;   //Changing the seconds by 1
                deltaMin = 0;   //Changing the minutes by 0 
                state = TIMER_CHANGE;   //Switch state to TIMER_CHANGE to update the minutes and seconds
            }
            else{   //If no buttons are pressed
                PB2Counter = 0; //Reset the PB2 counter as PB2 is not pressed
                state = NOTHING_PRESSED;    //Go back to NOTHING_PRESSED
            }


            break;
            
        case TIMER_CHANGE: //Handle the changing of the timer 
            changeTime(deltaSec, deltaMin); //Handles changing the seconds and minutes with values defined in BUTTON_PRESSED
            sendMessage("SET ");    //Send updated time
            delay_ms(200);    //Pause the code here so the program is more controllable by the user
            
            if(PB2Counter >= 10){   //Checks if PB2 is held for 2 seconds
                deltaSec = 5;   //Start adding 5 seconds per loop
            }
            else{
                PB2Counter++;   //Add to counter so after 2 seconds starts going up by 2
            }
            break;
        case TIMER_COUNTDOWN:    //Code to handle when timer3 causes the interrupt
            TMR3 = 0;            //Reset the timer counter
            T3CONbits.TON = 1;   //Re-enable the timer and start counting
            LEDOUT = !LEDOUT;   //Switch light to opposite state 
            changeTime(-1,0);   //Take away a second from the timer
            if(seconds == 0 && minutes == 0){   //Checks if the timer is done
                state = TIMER_COMPLETED;        //Change state
            }
            else{   //If not done continue countdown
                sendMessage("CNT ");    //Send message of current timer
                state = TIMER_IDLE;     //Go back to idle to wait for another interrupt
            }

            break;
        case TIMER_PAUSED:  //For when timer is paused and waiting for user input
            Idle();
            break;
        case TIMER_COMPLETED:   //When the timer reaches zero
            timerActive = 0;    //Disable timer active flag
            T3CONbits.TON = 0;  //Disable timer3
            LEDOUT = 1;         //Set LED to on
            sendMessage("FIN ");    //Send finish message
            timerPaused = 0;        //reset paused flag
            CNEN1bits.CN0IE = 1;    //Enable CN interrupt for PB1
            CNEN1bits.CN1IE = 1;    //Enable CN interrupt for PB2
            Idle();                 //Wait for user input
            break;
        case TIMER_IDLE:    //For when timer is idle waiting for a CN or timer interrupt
            Idle();
            break;
    }
}
    

void startTimer(){  //Starts the timer
    //T3CON config
    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
    T3CONbits.TCKPS = 1; // set prescaler to 1:8
    T3CONbits.TCS = 0; // use internal clock
    T3CONbits.TSIDL = 0; //operate in idle mode
    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; //enable timer interrupt
    PR3 = (31250); // set the count value for 1 s (or 1000 ms)
    TMR3 = 0;
    T3CONbits.TON = 1;  //Enable timer
    timerActive = 1;    //Set flag to on
    
    state = TIMER_IDLE;     //Send it into idle waiting for interrupt
    //Disable CN interrupts to prevent unwanted inputs
    CNEN1bits.CN0IE = 0;    //Disable CN interrupt for PB1
    CNEN1bits.CN1IE = 0;    //Disable CN interrupt for PB2
    
}
void sendMessage(char* message){
    char strTime[20];
    
    sprintf(strTime, "%02dm : %02ds", minutes, seconds); //Put minutes and seconds into a string with proper fromat
    
    char fullMessage[50];   //Array to handle the full message
    
    strcpy(fullMessage, message);   //Copy the beginning of the message defined in the argugment
    strcat(fullMessage, strTime);   //Add on the minutes and seconds
    
    if(state == TIMER_COMPLETED){   //When timer is completed add on an end part
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
    //T3CONbits.TON = 0;      //Disable timer 3
    IFS1bits.CNIF = 0;     //Clear the CN interrupt flag
}

// Timer 3 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!
    IFS0bits.T3IF = 0;  //Clear flag
    T3CONbits.TON = 0;  //Disable Timer3
    state = TIMER_COUNTDOWN;    //Set state to handle the timer3 interrupt 
}


void changeTime(int8_t changeSec, int8_t changeMinute){
    seconds += changeSec; //Change the seconds according to the argument
    
    //Checks if seconds overflowed if it was subtracted by 1
    //Checks if minutes are still present
    //Checks if you are changing the seconds
    //This to handle when seconds reaches 0 when there is still minutes
    if(seconds == 255 && minutes && changeSec){
        seconds = 59;   //Set to 59 if we reached a new minute
        minutes--;      //Take away a minute
    }
    else{
        minutes += changeMinute;    //Add to mintues

    }
    
    //If seconds overflow 60
    if(seconds >= 60){
        seconds = seconds % 60;   //Find remainder of seconds     
        minutes++;                //Add to minutes
    }

    if(minutes >= 60){
        minutes = 0;    //Set minutes to 0 is overflowed over 60
    } 
}

