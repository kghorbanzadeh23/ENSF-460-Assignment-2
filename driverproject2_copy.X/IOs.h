/*
 * File:  IOs.h
 * Author: Spiro, Kamand, Hutton
 *
 * Created on: 2024/10/31
 */

#ifndef IOs_H
#define IOs_H

#include <xc.h>

// Function declarations
void IOinit();
void IOcheck();
void sendMessage();
void StateInit();
void ShutOffTimers();
void SetPWM();
void IdleCheck();
void ResetClicked();
void AddToUARTTimer(uint8_t timeAdd);
#endif // TIMEDELAY_H