# ADC Reader - Project 2

## Overview
This project involves programming a microcontroller to control an LED's intensity and mode of operation using three push buttons (PB1, PB2, PB3) and a potentiometer. The system will also transmit data to a PC over UART, where a Python script will capture and plot the data.

## User Inputs and Outputs

### PB1 - System ON/OFF Mode
- **Input**: Click PB1 (no need to hold down the button).
- **Output**: 
  - If PB1 is clicked, the system enters ON MODE:
    - The LED turns on.
    - Turning the potentiometer adjusts the LED intensity between full brightness and 0% using Pulse Width Modulation (PWM).
  - If PB1 is clicked again, the system enters OFF MODE:
    - The LED turns off.
    - The system enters low-power Idle() mode.

### PB2 - LED Blinking Mode
- **Input**: Click PB2 (no need to hold down the button).
- **Output**:
  - While in ON MODE:
    - If PB2 is clicked, the LED blinks at approximately 500 ms intervals (0.5 s on, 0.5 s off) at the current intensity level.
    - Turning the potentiometer adjusts the intensity while blinking.
  - While in OFF MODE:
    - If PB2 is clicked, the LED blinks at 100% intensity level at approximately 500 ms intervals (0.5 s on, 0.5 s off).
  - Clicking PB2 again stops the LED blinking in either mode.

### PB3 - UART Transmission Mode
- **Input**: Click PB3 (no need to hold down the button).
- **Output**:
  - If PB3 is clicked during ON MODE (non-blink) or ON MODE (blink):
    - The microcontroller starts transmitting the intensity level and ADC reading to a PC over UART.
    - To start data collection simply run the ADCReader.py file and adjust ADC value by twisting potentiometer while data is being collected.
    - The Python script on the PC captures and stores the data in a CSV file for 1 minute, with appropriate timestamps.
    - After 1 minute, the script generates two graphs:
      1. ADC reading vs. time.
      2. Intensity levels (0-100%) vs. time.
  - Clicking PB3 again or leaving ON MODE stops the UART transmissions.
  - While transmitting, the user can switch between blink and non-blink modes, and the transmitted data/graphs will reflect the transitions.

## How to Operate
1. **Running the Python Script**: Execute the Python script on your PC to start capturing UART data.
2. **Using the System**:
   - Click PB1 to toggle the system between ON and OFF modes.
   - Click PB2 to toggle LED blinking in ON or OFF modes.
   - Click PB3 to start/stop UART data transmission and capture the data for plotting.

Ensure all connections are secure and the microcontroller is powered on before operating the system.