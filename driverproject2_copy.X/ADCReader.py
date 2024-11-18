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
ser = serial.Serial(port= "COM5", baudrate = 9600, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)


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
combined_df = pd.merge(
    dF[['Rx Time (sec)', 'ADC Value']],
    dF2[['Rx Time (sec)', 'Rx Intensity']],
    on='Rx Time (sec)',
    how='inner'  # Inner join ensures only matching times are included
)

# Rename columns for clarity
combined_df.rename(
    columns={
        'Rx Time (sec)': 'Time (sec)',
        'ADC Value': 'ADC Reading',
        'Rx Intensity': 'Intensity (Duty Cycle %)'
    }, 
    inplace=True
)

# Save the combined dataframe to a CSV file
combined_df.to_csv('Test_Run_Data.csv', index=False)

fig = make_subplots(rows=1, cols=2, subplot_titles=("ADC Reading (Raw)", "LED Intensity"))

trace1 = go.Scatter(x=dF['Rx Time (sec)'],y=dF['ADC Value'], name="ADC Reading (Raw)") 
trace2 = go.Scatter(x=dF2['Rx Time (sec)'], y=dF2['Rx Intensity'], name="LED Intensity")

fig.add_trace(trace1, row=1, col=1)
fig.add_trace(trace2, row=1, col=2)

fig.update_layout(height = 720, width = 1400, title_text = "Side by Side Subplots")

# Update axis labels for each subplot
fig.update_xaxes(title_text="Time (sec)", row=1, col=1)
fig.update_yaxes(title_text="ADC Reading", row=1, col=1)

fig.update_xaxes(title_text="Time (sec)", row=1, col=2)
fig.update_yaxes(title_text="Intensity (Duty Cycle %)", row=1, col=2)

fig.show()


