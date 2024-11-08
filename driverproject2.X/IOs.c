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
#define PB2 PORTBbits.RB4
#define PB3 PORTAbits.RA4
#define LEDOUT LATBbits.LATB8
#define PWMCYCLE 500

typedef enum{
    OFFMODE,   
    ONMODE,
    ONMODEBLINKING,
    OFFMODEBLINKING
} states_t;

uint8_t PB1Pressed = 0;
uint8_t PB2Pressed = 0;
uint8_t PB3Pressed = 0;

uint8_t PB1Clicked = 0;
uint8_t PB2Clicked = 0;
uint8_t PB3Clicked = 0;

extern int Delay_Flag;
states_t state; //Keeps track of state
uint16_t ADCvalue;
uint16_t previousADCvalue = 1;
double brightness = 0;
uint16_t PWMOnTime = 0;
uint16_t PWMOffTime = 0;

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
    state = OFFMODE;            
    
}

void sendMessage(char* message){
    char clear[61] = "\033[2J \033[H "; //Clears terminal
    strcat(clear, message);             //Add on message after clear
    Disp2String(clear);                 //Display whole message
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){    
    IFS1bits.CNIF = 0;     //Clear the CN interrupt flag

    
    if(!PB1){
        PB1Pressed = 1;
    }
    if(!PB2){
        PB2Pressed = 1;
    }
    if(!PB3){
        PB3Pressed = 1;
    }
    if(PB1 && PB1Pressed){
        PB1Pressed = 0;
        PB1Clicked = 1;
    }
    if(PB2 && PB2Pressed){
        PB2Pressed = 0;
        PB2Clicked = 1;
    }
    if(PB3 && PB3Pressed){
        PB3Clicked = 1;
    }
}

void ResetClicked(){
    PB1Clicked = 0;
    PB2Clicked = 0;
    PB3Clicked = 0;
}

void IdleCheck(){
    Idle();
    if(PB1Pressed || PB2Pressed || PB3Pressed){
        Idle();
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
    if(LEDOUT){
        PR3 = (PWMCYCLE * (ADCvalue * 0.0009766)) + 1; // set the count value for 1 s (or 1000 ms)
    }
    else{
        PR3 = (PWMCYCLE * (1 - (ADCvalue * 0.0009766))) + 1; // set the count value for 1 s (or 1000 ms)
    }
    TMR3 = 0;

    T3CONbits.TON = 1;  //Enable timer
}

// Timer 3 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!
    IFS0bits.T3IF = 0;  //Clear flag
    T3CONbits.TON = 0;  //Disable Timer3
    if(LEDOUT || brightness == 0){
        LEDOUT = 0;
        PR3 = (PWMCYCLE * (1 - brightness)) + 1;
        
    }
    else if(!LEDOUT){
        LEDOUT = 1;
        PR3 = PWMCYCLE * brightness + 1;
    }
    TMR3 = 0;
    T3CONbits.TON = 1;  //Enable Timer3
}


void ShutOffTimers(){
    T3CONbits.TON = 0;  //Enable timer
    T2CONbits.TON = 0;
}

void IOcheck(){
    
    switch(state){
        case OFFMODE:
            newClk(32);
            LEDOUT = 0;  
            ShutOffTimers();
            ADCvalue = do_ADC();
            Disp2Dec(ADCvalue);

            IdleCheck();
            
            if(PB1Clicked){
                newClk(500);
                ADCvalue = do_ADC();
                brightness = ADCvalue * 0.0009766;
                SetPWM();
                state = ONMODE;
                ResetClicked();
                break;
            }
            else if(PB2Clicked){
                newClk(500);
                state = OFFMODEBLINKING;
                ResetClicked();
                break;
            }
            break;
        case ONMODE:
            ADCvalue = do_ADC();
            brightness = ADCvalue * 0.0009766;
            Disp2Dec(PR3);
            IdleCheck();
            
            if(PB1Clicked){
                state = OFFMODE;
                ShutOffTimers();
                ResetClicked();
                break;
            }
            else if(PB2Clicked){
                state = ONMODEBLINKING;
                ResetClicked();
                break;
            }
            
            break;
        case ONMODEBLINKING:
            if(brightness == 0){
                ADCvalue = do_ADC();
                brightness = ADCvalue * 0.0009766;
            }
            else{
                brightness = 0;
            }
            delay_ms(500);
            while(!Delay_Flag && !PB1Clicked && !PB2Clicked && !PB3Clicked){
                IdleCheck();
            }
            
            if(PB1Clicked){
                state = OFFMODEBLINKING;
                ShutOffTimers();
                ResetClicked();
                
                if(brightness == 0)
                {
                    LEDOUT = 1;
                }
                else{
                    LEDOUT = 0;
                }

            }
            else if(PB2Clicked){
                state = ONMODE;
                ResetClicked();
            }
            break;
        case OFFMODEBLINKING:
            
            LEDOUT = !LEDOUT;
            delay_ms(500);
            while(!Delay_Flag && !PB1Clicked && !PB2Clicked && !PB3Clicked){
                IdleCheck();
            }
            if(PB1Clicked){
                state = ONMODEBLINKING;
                SetPWM();
                if(LEDOUT){
                    brightness = 1;
                }
                else{
                    brightness = 0;
                }
                ResetClicked();
            }
            else if(PB2Clicked){
                state = OFFMODE;
                ShutOffTimers();
                ResetClicked();
            }
            
            break;
    }
}