# -*- coding: utf-8 -*-
"""
ADC Reader - Project 2
Created on Friday Nov 8, 2024

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
from plotly.subplots import make_subplots
import plotly.graph_objects as go


## IMPORTS BELOW ONLY NEEDED IF USING SPYDER IDE
import plotly.io as pio  # needed to plot plotly graphs in spyderimport plotly.express as px

# pio.renderers.default = 'svg'  # to plot plotly graphs in spyder
pio.renderers.default = 'browser' # to plot plotly graphs in browser


## OPEN SERIAL PORT 
ser = serial.Serial(port= "COM4", baudrate = 9600, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)


## INITIALIZATIONS
rxADCStr = ''      #string to store received uint16_t numbers 
rxADCList = []      #List to store received uint16_t numbers in int form 
rxTimesList = []   #list to store time stamps of received uint16_t numbers
IntensityList = []
startTime = time.time()   

## CAPTURE UART DATA
while(time.time() - startTime < 5):  #record data for 1 sec
    line =ser.readline() # reads uint16_t nums as single bytes till \n n stores in string
    if ((line != b' \n') and (line != b'\n')) : #removes any '\n' without num captures
        rxADCStr = rxADCStr + line.decode('Ascii')  # Converts string of received uint16_t num to ASCII and combines Rx nums into 1 string
        timeMeas = time.time() -startTime # Time stamp received number
        rxTimesList.append(timeMeas) #save time stamps in a list
        print(rxADCStr)
        # print(rxTimesList)

        

## CLOSE SERIAL PORT    
ser.close()  # close any open serial ports


### Rx DATA CLEANUP AND STRING TO FLOAT CONVERSION
### Rx DATA CLEANUP AND STRING TO FLOAT CONVERSION
rxADCStr = rxADCStr.replace('\x00', '')  # Remove null characters

lines = rxADCStr.strip().split('\n')

for line in lines:
    if line:  # Ensure the line is not empty
        parts = line.split('.')
        if len(parts) == 2:
            rxADCList.append(int(parts[0]))
            IntensityList.append(int(parts[1]))

# print(rxADCList)
# print(IntensityList)

### CONVERT Rx DATA INTO DATA FRAME
dF = pd.DataFrame()
dF['Rx Time (sec)'] = rxTimesList
dF['ADC Value'] = rxADCList

dF2 = pd.DataFrame()
dF2['Rx Time (sec)'] = rxTimesList
dF2['Rx Intensity'] = IntensityList

### DATA STATISTICS
# print(dF.describe())
# print(dF2.describe())


### COPY RX VOLTAGE AND RX TIME IN CSV AND XLS FILES
dF.to_csv('RxDataFloat.csv', index = True)
dF2.to_csv('RxDataFloat2.csv', index = True)

# dF.to_excel('RxDataFloat.xlsx', sheet_name='New Sheet')

fig = make_subplots(rows=1, cols=2)

fig.add_trace(
    go.Scatter(x=dF['Rx Time (sec)'],y=dF['ADC Value'], dx = 1, dy = 1) 
    
)


fig.add_trace(
    go.Scatter(x=dF2['Rx Time (sec)'], y=dF2['Rx Intensity'],  dx = 1, dy = 2)
)



### PLOT Rx VOLTAGE VS Rx TIME
# fig = px.line(dF, x='Rx Time (sec)', y='ADC Value', title = 'ADC vs Time')
# fig2 = px.line(dF2, x='Rx Time (sec)', y='Rx Intensity', title = 'Intensity vs Time')

fig.update_layout(height = 600, width = 800, title_text = "2 Subplots")

fig.show()
# fig2.show()
# fig.write_image("image1.png")

