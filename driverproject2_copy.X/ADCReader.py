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

# Imports used for data collection and graph creation
import pandas as pd
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.io as pio  

pio.renderers.default = 'browser' # to plot plotly graphs in browser


## OPEN SERIAL PORT
ser = serial.Serial(port= "COM5", baudrate = 9600, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)


## INITIALIZATIONS
rxADCStr = ''      # String to store received brigthness and ADC values in the form 'num'.'num' 
rxADCList = []     # List to store received ADC values in float form 
rxTimesList = []   # List to store time stamps while transferring over UART
IntensityList = [] # List to store recieved brightness (intensity) values as a percentage from 1-100
startTime = time.time()   

## CAPTURE UART DATA
while(time.time() - startTime < 5):  # record data for 5 sec
    line =ser.readline() # reads uint16_t nums as single bytes till \n n stores in string
    if ((line != b' \n') and (line != b'\n')) : # removes any '\n' without num captures
        rxADCStr = rxADCStr + line.decode('Ascii')  # Converts string of received uint16_t num to ASCII and combines Rx nums into 1 string
        timeMeas = time.time() -startTime # Time stamp received number
        rxTimesList.append(timeMeas) # Save time stamps in a list
        print(rxADCStr)

## CLOSE SERIAL PORT    
ser.close()  # close any open serial ports

### Rx DATA CLEANUP AND STRING TO FLOAT CONVERSION
rxADCStr = rxADCStr.replace('\x00', '')  # Remove null characters

# Split the received ADC string into individual lines based on newline character
lines = rxADCStr.strip().split('\n')

# Iterate over each line in the split lines
for line in lines:
    if line:  # Ensure the line is not empty
        parts = line.split('.')  # Split the line into ADC value and intensity based on the '.' delimiter
        if len(parts) == 2:
            rxADCList.append(int(parts[0]))        # Append the first number to the ADC values list
            IntensityList.append(int(parts[1]))    # Append the second number to the intensity list

### CONVERT Rx DATA INTO DATA FRAME
dF = pd.DataFrame()                     # Create an empty DataFrame for ADC values
dF['Rx Time (sec)'] = rxTimesList       # Add the received timestamps to the DataFrame
dF['ADC Value'] = rxADCList             # Add the ADC values to the DataFrame

dF2 = pd.DataFrame()                    # Create a second empty DataFrame for intensity values
dF2['Rx Time (sec)'] = rxTimesList      # Add the received timestamps to the second DataFrame
dF2['Rx Intensity'] = IntensityList     # Add the intensity values to the second DataFrame

### DATA STATISTICS
# print(dF.describe())
# print(dF2.describe())
### COPY RX VOLTAGE AND RX TIME IN CSV AND XLS FILES

# Merge the two DataFrames on the 'Rx Time (sec)' column using an inner join.
# This ensures that only the rows with matching timestamps in both DataFrames are included.
combined_df = pd.merge(
    dF[['Rx Time (sec)', 'ADC Value']],       # Select 'Rx Time (sec)' and 'ADC Value' from the first DataFrame
    dF2[['Rx Time (sec)', 'Rx Intensity']],  # Select 'Rx Time (sec)' and 'Rx Intensity' from the second DataFrame
    on='Rx Time (sec)',                       # Specify the column to join on
    how='inner'  # Inner join ensures only matching times are included
)

# Rename the columns of the combined DataFrame for better clarity and readability.
combined_df.rename(
    columns={
        'Rx Time (sec)': 'Time (sec)',            # Rename 'Rx Time (sec)' to 'Time (sec)'
        'ADC Value': 'ADC Reading',               # Rename 'ADC Value' to 'ADC Reading'
        'Rx Intensity': 'Intensity (Duty Cycle %)' # Rename 'Rx Intensity' to 'Intensity (Duty Cycle %)'
    }, 
    inplace=True  # Apply the changes directly to the combined_df without creating a new DataFrame
)

# Save the combined DataFrame to a CSV file named 'Test_Run_Data.csv'.
# The index is set to False to exclude the DataFrame index from the CSV file.
combined_df.to_csv('Test_Run_Data.csv', index=False)

# Create a subplot layout with 1 row and 2 columns for side-by-side plots.
# The subplot_titles parameter adds titles to each subplot.
fig = make_subplots(rows=1, cols=2, subplot_titles=("ADC Reading (Raw)", "LED Intensity"))

# Create a scatter plot for ADC readings over time.
trace1 = go.Scatter(
    x=dF['Rx Time (sec)'],            # Set the x-axis to 'Rx Time (sec)' from the first DataFrame
    y=dF['ADC Value'],                # Set the y-axis to 'ADC Value' from the first DataFrame
    name="ADC Reading (Raw)"          # Name of the trace for the legend
) 

# Create a scatter plot for LED intensity over time.
trace2 = go.Scatter(
    x=dF2['Rx Time (sec)'],           # Set the x-axis to 'Rx Time (sec)' from the second DataFrame
    y=dF2['Rx Intensity'],            # Set the y-axis to 'Rx Intensity' from the second DataFrame
    name="LED Intensity"              # Name of the trace for the legend
)

# Add the first trace to the first subplot (row 1, column 1).
fig.add_trace(trace1, row=1, col=1)

# Add the second trace to the second subplot (row 1, column 2).
fig.add_trace(trace2, row=1, col=2)

# Update the layout of the figure with specified height, width, and a main title.
fig.update_layout(
    height=720,                # Set the height of the plot in pixels
    width=1400,                # Set the width of the plot in pixels
    title_text="Side by Side Subplots"  # Main title for the entire figure
)

# Update the x-axis label for the first subplot.
fig.update_xaxes(
    title_text="Time (sec)",    # Label for the x-axis
    row=1,                      # Specify the subplot row
    col=1                       # Specify the subplot column
)

# Update the y-axis label for the first subplot.
fig.update_yaxes(
    title_text="ADC Reading",   # Label for the y-axis
    row=1,                      # Specify the subplot row
    col=1                       # Specify the subplot column
)

# Update the x-axis label for the second subplot.
fig.update_xaxes(
    title_text="Time (sec)",    # Label for the x-axis
    row=1,                      # Specify the subplot row
    col=2                       # Specify the subplot column
)

# Update the y-axis label for the second subplot.
fig.update_yaxes(
    title_text="Intensity (Duty Cycle %)",  # Label for the y-axis
    row=1,                                   # Specify the subplot row
    col=2                                    # Specify the subplot column
)

# Display the figure with the configured subplots and traces.
fig.show()

