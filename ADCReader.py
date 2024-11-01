# -*- coding: utf-8 -*-
"""
ADC Reader - Assignment 4
Created on Thursday Oct 31, 2024

CAPTURING 16 BIT INT AND FLOAT NUMBERS OVER A TIME INTERVAL
Time stamps received numbers separated by \n
Saves in Dataframe
Saves in csv
Plots received data versus time

@author: Spiro Douvis, Kamand Ghorbanzadeh, Hutton Ledingham
"""

import numpy as np
import math 
import csv
import serial  # pip install pyserial  or conda install pyserial
import time
import pandas as pd
import plotly.express as px

## IMPORTS BELOW ONLY NEEDED IF USING SPYDER IDE
import plotly.io as pio  # needed to plot plotly graphs in spyder
# pio.renderers.default = 'svg'  # to plot plotly graphs in spyder
pio.renderers.default = 'browser' # to plot plotly graphs in browser


## OPEN SERIAL PORT 
ser = serial.Serial(port= "COM4", baudrate = 4800, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)


## INITIALIZATIONS
rxNumsStr = ''      #string to store received uint16_t numbers 
rxNumsList = []      #List to store received uint16_t numbers in int form 
rxTimesList = []   #list to store time stamps of received uint16_t numbers
voltNumsList = []
startTime = time.time()   

## CAPTURE UART DATA
while(time.time() - startTime < 10):  #record data for 1 sec
    line =ser.readline() # reads uint16_t nums as single bytes till \n n stores in string
    if ((line != b' \n') and (line != b'\n')) : #removes any '\n' without num captures
        rxNumsStr = rxNumsStr + line.decode('Ascii')  # Converts string of received uint16_t num to ASCII and combines Rx nums into 1 string
        timeMeas = time.time() -startTime # Time stamp received number
        rxTimesList.append(timeMeas) #save time stamps in a list
        # print(rxNumsStr)

## CLOSE SERIAL PORT    
ser.close()  # close any open serial ports

rxStr = rxNumsStr #checks
# print(rxStr)
# print(rxNumsStr)  
# print(rxTimesList)


### Rx DATA CLEANUP AND STRING TO FLOAT CONVERSION
rxNumsStr = rxNumsStr.replace('\x00','')  #\x00 seems to be sent with Disp2String()


rxNumsStr = rxNumsStr.strip() # remove unwanted chars and spaces 
rxNumsList = rxNumsStr.split(' \n')  # split string by \n n store in list
print(rxNumsList)
rxNumsList = [float(elem) for elem in rxNumsList]  # convert char in List into int
voltNumsList = [(elem) * 3/(2 ** 10) for elem in rxNumsList]

# print(rxNumsList)       #check
print(len(rxTimesList))
print(len(rxNumsList))


### CONVERT Rx DATA INTO DATA FRAME
dF = pd.DataFrame()
dF['Rx Time (sec)'] = rxTimesList
dF['Buffer Value'] = rxNumsList

dF2 = pd.DataFrame()
dF2['Rx Time (sec)'] = rxTimesList
dF2['Rx Voltage'] = voltNumsList

### DATA STATISTICS
print(dF.describe())
print(dF2.describe())


### COPY RX VOLTAGE AND RX TIME IN CSV AND XLS FILES
dF.to_csv('RxDataFloat.csv', index = True)
dF2.to_csv('RxDataFloat2.csv', index = True)

# dF.to_excel('RxDataFloat.xlsx', sheet_name='New Sheet')



### PLOT Rx VOLTAGE VS Rx TIME
fig = px.line(dF, x='Rx Time (sec)', y='Buffer Value', title = 'Buffer vs Time')
fig2 = px.line(dF2, x='Rx Time (sec)', y='Rx Voltage', title = 'Voltage vs Time')

fig.show()
fig2.show()
# fig.write_image("image1.png")

