/*
 * File:   IOs.c
 * Author: Spiro, Kamand, Hutton
 *
 * Created on November 4, 2024, 10:25 AM
 */


#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include "IOs.h"
#include "TimeDelay.h"
#include "clkChange.h"
#include "UART.h"
#include "ADC.h"

//Define for easier readability and changes
#define PB1 PORTAbits.RA2 
#define PB2 PORTBbits.RB4
#define PB3 PORTAbits.RA4
#define LEDOUT LATBbits.LATB8
#define PULSEPERIOD 9000
#define UARTTIMEOUT 40000

//Different states for our program
typedef enum{
    STATE_OFFMODE, //LED is off  
    STATE_ONMODE,     //LED is On according to the ADC value
    STATE_ONMODEBLINKING, //LED is blinking while the brightness is related to ADC value
    STATE_OFFMODEBLINKING //100% brightness blinking
} states_t;
//Global variables
char message[15];   //To hold the message to send over UART

uint8_t PB1Pressed = 0;     //To check which buttons were pressed
uint8_t PB2Pressed = 0;
uint8_t PB3Pressed = 0;

uint8_t PB1Clicked = 0;     //To check which ones were clicked
uint8_t PB2Clicked = 0;
uint8_t PB3Clicked = 0;

uint16_t UARTTimer = 0;     //Counter for the UART timeout

uint8_t UARTtransfer = 0;   //Flag for UARTtransfer
uint8_t percent = 0;        //Hold percent value

extern int Delay_Flag;      //Delay flag from TimeDelay.c
states_t state; //Keeps track of state
uint16_t ADCvalue;  //Holds ADC value
float brightness = 0;   //hold brightness value

void IOinit(){
    
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

}

void StateInit(){
    state = STATE_OFFMODE;            
    
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){    
    IFS1bits.CNIF = 0;     //Clear the CN interrupt flag

    //Check which PB is pressed
    if(!PB1){
        PB1Pressed = 1;
    }
    if(!PB2){
        PB2Pressed = 1;
    }
    if(!PB3){
        PB3Pressed = 1;
    }
    if(PB1 && PB1Pressed){  //If PB is not pressed and it was previously pressed
        PB1Pressed = 0;
        PB1Clicked = 1;
    }
    if(PB2 && PB2Pressed){
        PB2Pressed = 0;
        PB2Clicked = 1;
    }
    if(PB3 && PB3Pressed){
        PB3Pressed = 0;
        PB3Clicked = 1;
    }
}

void ResetClicked(){    //Resets all the clicked variables
    PB1Clicked = 0;
    PB2Clicked = 0;
    PB3Clicked = 0;
}

void IdleCheck(){   //Does the right amount of Idle's depending on the event
    Idle();     //Default Idle 
    if(PB1Pressed || PB2Pressed || PB3Pressed){ //To check for the click with another idle if any of the push buttons is pressed
        Idle();
    }

}

void AddToUARTTimer(uint8_t timeAdd){   //Add time to the counter to be able to stop the transfer
    UARTTimer += timeAdd;   //Add to the counter
    if(UARTTimer >= UARTTIMEOUT){   //Check if it reaches the timeout number
        UARTtransfer = 0;           //Turn off the transfer
        UARTTimer = 0;              //Reset timer counter
    }    
}

void SetPWM(){
    //T3CON config
    T2CONbits.T32 = 0; // operate timer 2 as 16 bit timer
    T3CONbits.TCKPS = 1; // set prescaler to 1:8
    T3CONbits.TCS = 0; // use internal clock
    T3CONbits.TSIDL = 0; //operate in idle mode
    IPC2bits.T3IP = 2; //7 is highest and 1 is lowest pri.
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1; //enable timer interrupt
    
    ADCvalue = do_ADC();
    brightness = ADCvalue * 0.0009775;
    LEDOUT = 1;
    PR3 = (PULSEPERIOD * brightness) + 1; // set the PR3 value for the time it is on

    TMR3 = 0;

    T3CONbits.TON = 1;  //Enable timer
}

// Timer 3 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!
    IFS0bits.T3IF = 0;  //Clear flag
    T3CONbits.TON = 0;  //Disable Timer3
    if(LEDOUT || brightness == 0){  //If the LED is on or brightness is zero
        LEDOUT = 0;                 //Turn LED off
        PR3 = (PULSEPERIOD * (1 - brightness)) + 1;    //Calculate time the LED is off
        
    }
    else if(!LEDOUT || brightness == 1){
        LEDOUT = 1; //Turn LED on
        PR3 = PULSEPERIOD * brightness + 1;            //Calculate time the LED is on
    }
    TMR3 = 0;           //Reset TMR3 counter
    T3CONbits.TON = 1;  //Enable Timer3
}

void FindBrightness(){
    ADCvalue = do_ADC();                //Grab ADC value

    if(ADCvalue == 1023)    //Check if ADC reached max value
        brightness = 1;  //Set brightness to max brightness
    else
        brightness = ADCvalue * 0.0009775;  //Find the brightness value with reciprocal of 1023
}

void ShutOffTimers(){
    T3CONbits.TON = 0;  //Disable timer 3
    T2CONbits.TON = 0;  //Disable timer 2
}

void OffMode(){
    newClk(32); //Set clock to 32 kHz
    LEDOUT = 0;  //Turn LED off
    ShutOffTimers();    //Shut all timers off 
    UARTTimer = 0;
    IdleCheck();    //Idle until button click
}

void OnMode(){
    FindBrightness();

    if(UARTtransfer){  //Do UART transfers
                percent = brightness * 100; //Find percent to send over UART 
                sprintf(message, "%d.%d \n", ADCvalue, percent);  //Put ADC and percent value in a string with correct formatting
                Disp2String(message);   //Send over UART
                AddToUARTTimer(15);     //Add to counter
    }    
    
    IdleCheck();    //Idle here until interrupt
}

void OffModeBlinking(){            
    LEDOUT = !LEDOUT;   //Turn off and on the LED
    delay_ms(500);      //Delay for 500ms
    while(!Delay_Flag && !PB1Clicked && !PB2Clicked && !PB3Clicked){    //Stay here until button clicked or delay is done
        IdleCheck();
    }            
}

void OnModeBlinking(){
    if(brightness){                   //If the LED was ON turn it off
        brightness = 0;
    }
    else{               //If LED was off turn it on again
        FindBrightness();
    }
    delay_ms(500);          //Delay for 500ms
    while(!Delay_Flag && !PB1Clicked && !PB2Clicked && !PB3Clicked){    //Only continue if delay is finished or PB is clicked
        if(brightness){ //If brightness is active check the value
            FindBrightness();
        }

        if(UARTtransfer){   //Check if UART transfer is active
            percent = brightness * 100; //Calc percent value
            sprintf(message, "%d.%d \n", ADCvalue, percent);  //Put ADC and Percent value in a string with correct formatting
            Disp2String(message);   //Send string over UART
        }
        IdleCheck();
    }
            
}
void IOcheck(){
    
    switch(state){
        case STATE_OFFMODE:
            OffMode();
            
            if(PB1Clicked){ //Checks if PB1 is clicked
                newClk(8);  //Set clock to 8 MHz
                SetPWM();   //Turn on PWM
                state = STATE_ONMODE; //Change state to ONMODE
                ResetClicked(); //Reset the click variables
                break;
            }
            else if(PB2Clicked){    //Check if PB2 is clicked
                newClk(8);          //Set clock to 8 MHz
                state = STATE_OFFMODEBLINKING;    //Change state to OFFMODEBLINKING
                ResetClicked();
                break;
            }
            break;
        case STATE_ONMODE:    //Relates ADC value to LED brightness
            OnMode();
            
            if(PB1Clicked){ //Check PB1Clicked
                state = STATE_OFFMODE;    //Change the state to OFFMODE
                ShutOffTimers();    //Shut off all the timers
                ResetClicked();     //Reset click variables
                break;
            }
            else if(PB2Clicked){
                state = STATE_ONMODEBLINKING; //Change the state to ONMODEBLINKING
                ResetClicked();
                break;
            }
            else if(PB3Clicked){
                UARTtransfer = !UARTtransfer;   //Enable or disable UARTtransfer
                UARTTimer = 0;                  //Reset Timer Counter
                ResetClicked(); 
                break;
            }

            
            break;
        case STATE_ONMODEBLINKING: //Blinking LED while reading from ADC value
            OnModeBlinking();
            if(PB1Clicked){     //Check if PB1 is clicked
                UARTTimer = 0;
                state = STATE_OFFMODEBLINKING;    //Go to OFF MODE BLINKING
                ShutOffTimers();            //Shut off all the timers
                ResetClicked();             //Reset the clicked variables
                
                if(!brightness)         //Check if the LED was off or on during the blinking
                {
                    LEDOUT = 1;             //Turn it on if it was off
                }
                else{
                    LEDOUT = 0;             //Turn it off if it was on
                }

            }
            else if(PB2Clicked){    //Check if PB2 is clicked
                state = STATE_ONMODE;     //Change state to ONMODE
                ResetClicked();
                break;
            }
            else if(PB3Clicked){    //Check if PB3 is clicked
                UARTtransfer = !UARTtransfer;   //Turn on or off the UARTtransfer
                UARTTimer = 0;                  //Reset counter
                ResetClicked();                 //Reset clicked variables
                break;
            }
            else if(UARTtransfer){      //If the UARTtransfer is active
                AddToUARTTimer(250);    //Add to Counter
            }
            break;
        case STATE_OFFMODEBLINKING:   //To handle regular blinking at 100% brightness
            OffModeBlinking();
            if(PB1Clicked){ //Check is PB1 is clicked
                state = STATE_ONMODEBLINKING; //Change state to ON MODE BLINKING
                SetPWM();   //Turn on the PWM
                if(!LEDOUT){ //If the LED is on
                    brightness = 1; //Make sure the LED stays on
                }
                else{
                    brightness = 0; //Make sure the LED turns off
                }
                ResetClicked();
            }
            else if(PB2Clicked){    //If PB2 Clicked
                state = STATE_OFFMODE;    //Go to OFF MODE
                ShutOffTimers();    //Turn off all timers
                ResetClicked();
            }
            
            break;
    }
}