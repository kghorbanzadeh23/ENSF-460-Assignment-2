/* 
 * File:   ADC.c
 * Author: spiro
 *
 * Created on October 28, 2024, 3:14 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "ADC.h"


/*
 * 
 */

uint16_t do_ADC(void)
{
    uint16_t ADCvalue ; // 16 bit register used to hold ADC converted digital output ADC1BUF0
    /* ------------- ADC SAMPLING AND CONVERSION ------------------*/
    AD1CON1bits.SAMP = 1; //Start Sampling, Conversion starts automatically after SSRC and SAMC settings
//    AD1CON1bits.ADON = 1; // turn on ADC module
    while(AD1CON1bits.DONE==0)
        {}
    ADCvalue = ADC1BUF0; // ADC output is stored in ADC1BUF0 as this point
    AD1CON1bits.SAMP = 0; //Stop sampling
//    AD1CON1bits.ADON = 0; //Turn off ADC, ADC value stored in ADC1BUF0;
    return (ADCvalue); //returns 10 bit ADC output stored in ADC1BIF0 to calling function
}

uint16_t ADC_init(void)
{
    /* ------------- ADC INITIALIZATION ------------------*/
    TRISAbits.TRISA3 = 1;   //Set pin 9 to input
    AD1PCFG = 0xFFFF; // I/O pins that can also be analog to be digital 
    AD1CON1bits.SSRC = 7; // Internal Counter ends sampling and starts conversion
    AD1CON1bits.FORM = 0;   //Integer Form for ouput
    AD1CON1bits.ASAM = 0; //Sampling starts when SAMP bit is set
    AD1CON1bits.ADON = 1; //Turn off ADC, ADC value stored in ADC1BUF0;
    AD1CON1bits.DONE = 1;   
    AD1CON2bits.VCFG = 0b000; // Selects AVDD, AVSS (supply voltage to PIC) as Vref
    AD1CON2bits.CSCNA = 0; // Do not scan inputs
    AD1CON2bits.SMPI = 0;   //Interrupts at completion of conversion to each sample
    AD1CON2bits.BUFM = 0;   //Buffer configured as one 16 word buffer
    AD1CON2bits.ALTS = 0;   //Always uses MUX A input multiplexer settings
    AD1CON3bits.ADRC = 0; // Use system clock
    AD1CON3bits.SAMC = 0x7; //Set to Slowest sampling rate time = 21*2/fclk
    AD1CHSbits.CH0SA = 5; // Select and configure ADC input
    AD1CHSbits.CH0NA = 0; // Select and configure ADC input
    AD1PCFGbits.PCFG5 = 0;  //Pin configured in Analog mode, I/O port read disabled
    AD1CSSLbits.CSSL5 = 0;  //Analog channel omitted from input scan
}