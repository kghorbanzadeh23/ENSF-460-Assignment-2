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
    TRISAbits.TRISA3 = 1;
    AD1PCFG = 0xFFFF;
    AD1CON1bits.SSRC = 7; // Configure ADC by setting bits in AD1CON1 register
    AD1CON1bits.FORM = 0;
    AD1CON1bits.ASAM = 0; 
    AD1CON1bits.ADON = 1; //Turn off ADC, ADC value stored in ADC1BUF0;
    AD1CON1bits.DONE = 1;
    AD1CON2bits.VCFG = 0b000; // Selects AVDD, AVSS (supply voltage to PIC) as Vref
    AD1CON2bits.CSCNA = 0; // Configure ADC by setting bits in AD1CON2
    AD1CON2bits.SMPI = 0;
    AD1CON2bits.BUFM = 0;
    AD1CON2bits.ALTS = 0;
    AD1CON3bits.ADRC = 0; // Use system clock
    AD1CON3bits.SAMC = 0x7; //Configure the ADC?s sample time by setting bits in AD1CON3
    AD1CHSbits.CH0SA = 5; // Select and configure ADC input
    AD1CHSbits.CH0NA = 0; // Select and configure ADC input
    AD1PCFGbits.PCFG5 = 0;
    AD1CSSLbits.CSSL5 = 0;
}