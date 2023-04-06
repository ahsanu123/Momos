# -*- coding: utf-8 -*-
"""
Created on Sun Mar 26 04:49:08 2023

@author: Ahsanu amala
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import datetime as date
import serial
import serial.tools.list_ports as serPorts
from multiprocessing import Process
import queue 
import threading
import time


import gradio as gr


# ==========================================================================
# Variable section
# ==========================================================================
serialGlobal=serial
ser=serial.Serial()
boolPrintSerial=False

# ==========================================================================
# Class section
# ==========================================================================

class ReadLine:
    def __init__(self, s):
        self.buf = bytearray()
        self.s = s
    
    def readline(self):
        i = self.buf.find(b"\n")
        if i >= 0:
            r = self.buf[:i+1]
            self.buf = self.buf[i+1:]
            return r
        while True:
            i = max(1, min(2048, self.s.in_waiting))
            data = self.s.read(i)
            i = data.find(b"\n")
            if i >= 0:
                r = self.buf + data[:i+1]
                self.buf[0:] = data[i+1:]
                return r
            else:
                self.buf.extend(data)
                

# ==========================================================================
# Function section
# ==========================================================================

def serialPrint(inqueue):
    global ser
    print("Hallow??")
    while True:
        #buf = str(ser.readline())
        inqueue.put("buf")

def startProcess():
    #set_start_method("spawn")
    Process(target=serialPrint).start()
    return 

def listPort():
    buffer=''
    for port in sorted(serPorts.comports()):
        buffer+=str(port) + "\n"
    return buffer
        

def connectPort(port, baudrate):
    global ser
    global boolPrintSerial
    global getDataBtn
    
    
    if port=='' or baudrate=='': 
        return "## ⚠ Select Port and Baudrate!!!"
    try:
        ser = serial.Serial(port=port, baudrate=baudrate)
        ser=ser
        
    except serial.serialutil.SerialException as e:
        boolPrintSerial=False
        return "❌ port NOT Connected: " + str(port) + str(e) 
    
    
    boolPrintSerial=True
    print(ser.is_open)
    return "✅ port Connected: " + str(port) + "\ninfo: " + str(ser) 

def plot_forecast(final_year, companies, noise, show_legend, point_style):
    start_year = 2020
    x = np.arange(start_year, final_year + 1)
    year_count = x.shape[0]
    plt_format = ({"cross": "X", "line": "-", "circle": "o--"})[point_style]
    fig = plt.figure()
    ax = fig.add_subplot(111)
    for i, company in enumerate(companies):
        series = np.arange(0, year_count, dtype=float)
        series = series**2 * (i + 1)
        series += np.random.rand(year_count) * noise
        ax.plot(x, series, plt_format)
    if show_legend:
        plt.legend(companies)
    return fig

def plot_random():
    y = np.random.rand(100)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.plot(range(0,100), y)
    return fig

"""
TODO: 
    1. buat fungsi dengan multiprocessing untuk mengambil data dari serial
    2. buat fungsi untuk melakukan plot, preprocessing dan menampilkan data input untuk inference
    3. buat plot untuk menampilkan hasil kemungkinan dari model
    4. buat fungsi untuk menampilkan hasil klasifikasi (sehat,BRB,dll)
    5. buat fungsi untuk setting, seperti com port baudrate dsb
    6. [optional] buat fungsi untuk menampilkan informasi model 
    7. buat fungsi untuk menampilkan hasil pembacaan dari sensor
    8. [optional] buat fungsi untuk menampilkan history request dari modbus
    9. buat fungsi untuk mengganti seting modbus
"""


# ==========================================================================
# Block user interface
# ==========================================================================

bl = gr.Blocks()
with bl:
    gr.Markdown("# MoMos - Motor Monitoring System Dashboard")    
    with gr.Tab("Setting"):
        with gr.Column():
            gr.Markdown("## Port Settings: ")
            portListMd = gr.Markdown()
            
            port = gr.Textbox(max_lines=1, label="insert port")
            baudrate = gr.Radio([19200, 38400, 57600, 115200, 128000 ],label="choose baudrate")
            
            listPortBtn = gr.Button(value="List Port")
            serialBtn = gr.Button(value="Connect")
            getDataBtn = gr.Button(value="Get Data")
            
            
            portConStatus = gr.Markdown()
            portConnectionStatus = gr.Markdown()
            
            serialBtn.click(connectPort, inputs=[port,baudrate], outputs=portConnectionStatus)
            listPortBtn.click(listPort, outputs=portListMd)
            getDataBtn.click(startProcess)
            
            
            # modbus section 
            gr.Markdown("## modbus setting: ")
            portRadio = gr.Radio(['1', '2', '3', '4', '5'], label="Choose Modbus NUM")
            gr.Textbox(max_lines=1, label="MODBUS NUM: ", placeholder="Choose or Insert")
            
    with gr.Tab("Display"):
        with gr.Column():
            gr.Markdown("### display plot Here!!")
            year = gr.Radio([2025, 2030, 2035, 2040], label="Project to:")
            group = gr.CheckboxGroup(["Google", "Microsoft", "Gradio"], label="Company Selection")
            noise = gr.Slider(1, 100, label="Noise Level")
            showLegend = gr.Checkbox(label="Show Legend")
            style = gr.Dropdown(["cross", "line", "circle"], label="Style")
            
            showPlot = gr.Button("Plot")
            
            plotPlace = gr.Plot(label="forecast")
            
            showPlot.click(plot_forecast,inputs=[year,group,noise,showLegend,style],outputs=plotPlace)
            
    with gr.Tab("Clasification"):
        gr.Markdown("# Do Clasification here")
        
        with gr.Row():
            with gr.Column():
                gr.Markdown("## show current image data here")
                randomPlot = gr.Plot(label="current data")
                gr.Markdown("## "+str(date.datetime.now()))
                
                bl.load(plot_random,outputs=randomPlot)
                
            with gr.Column():
                gr.Markdown("## display prediction bar here")
                predictionBar = gr.Plot()
                predictionResult = gr.Plot()
                bl.load(plot_random,outputs=predictionBar)
                bl.load(plot_random,outputs=predictionResult)
                
            
def startGradio():
    bl.launch(server_port=8070, debug=True)
    
def main():
    
    global serialGlobal
    global ser
    
    ser = serialGlobal.Serial(port='com9', baudrate=115200)
    
    serialQueue = queue.Queue()
    serialThread = threading.Thread(target=serialPrint, args=(serialQueue), daemon=True)
    serialThread.start()
    
    #gradioThread = threading.Thread(target=startGradio, daemon=True)
    #gradioThread.start()
    
    while(True):
        #print(serialQueue.qsize())
        if serialQueue.qsize()>0:
            print(str(serialQueue.get()))
        time.sleep(0.01)
    
    print("until DOWN!!")

main()