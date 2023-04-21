# -*- coding: utf-8 -*-
"""
Created on Sun Mar 26 04:49:08 2023

@author: Ahsanu amala
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import serial
import serial.tools.list_ports as serPorts
import threading
import glob
# from multiprocessing import Process
# import queue 

import time
import schedule # not built in library
import os
import signal
import subprocess
from datetime import datetime
import psutil

import gradio as gr


# ==========================================================================
# Class section
# ==========================================================================

# class ReadLine:
#     def __init__(self, s):
#         self.buf = bytearray()
#         self.s = s
    
#     def readline(self):
#         i = self.buf.find(b"\n")
#         if i >= 0:
#             r = self.buf[:i+1]
#             self.buf = self.buf[i+1:]
#             return r
#         while True:
#             i = max(1, min(2048, self.s.in_waiting))
#             data = self.s.read(i)
#             i = data.find(b"\n")
#             if i >= 0:
#                 r = self.buf + data[:i+1]
#                 self.buf[0:] = data[i+1:]
#                 return r
#             else:
#                 self.buf.extend(data)
                

# ==========================================================================
# Function section
# ==========================================================================

# def serialPrint(inqueue):
#     global ser
#     print("Hallow??")
#     while True:
#         #buf = str(ser.readline())
#         inqueue.put("buf")

# def startProcess():
#     #set_start_method("spawn")
#     Process(target=serialPrint).start()
#     return 

# def connectPort(port, baudrate):
#     global ser
#     global boolPrintSerial
#     global getDataBtn
    
    
#     if port=='' or baudrate=='': 
#         return "## ⚠ Select Port and Baudrate!!!"
#     try:
#         ser = serial.Serial(port=port, baudrate=baudrate)
#         ser=ser
        
#     except serial.serialutil.SerialException as e:
#         boolPrintSerial=False
#         return "❌ port NOT Connected: " + str(port) + str(e) 
    
    
#     boolPrintSerial=True
#     print(ser.is_open)
#     return "✅ port Connected: " + str(port) + "\ninfo: " + str(ser) 

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

def listPort():
    buffer=''
    for port in sorted(serPorts.comports()):
        buffer+=str(port) + "\n"
    return buffer

def getRawData(com):
    currentProcess = psutil.Process()
    children = currentProcess.children(recursive=True)
    
    # if still have another process, kill it
    if len(children)>0:
        for child in children:
            os.kill(child.pid, signal.SIGTERM)
            
    # execute operation
    dir = os.path.dirname(__file__)
    filename = str(datetime.today().strftime('%Y_%m_%d__%H%M%S')) + ".csv"
    filename = os.path.join(dir, filename)
    print(f'Create File: {filename}')
    file = open(filename, "w+")
    
    command = ["cat", com]
    p = subprocess.Popen(command, stdout=file)
    print(f'{p}\t{p.pid}')        
    return f'{p}\t{p.pid}'
    
def startSchedule(t, com):
    
    print(f't: {int(t)}, com: {com}')
    if com == '' or t == '': 
        return
    
    schedule.clear()
    schedule.every(int(t)).seconds.do(getRawData,com=com)
    return "schedule created"

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
            
            with gr.Row():
                port = gr.Textbox(max_lines=1, label="insert port")
                intime = gr.Number(label="Every", value=5, interactive=True)
            
            
            startStatus = gr.Markdown(value="yoyoyo")
            with gr.Row():
                listPortBtn = gr.Button(value="List Port")
                startBtn = gr.Button(value="Start")
                
                listPortBtn.click(listPort, outputs=portListMd)
                startBtn.click(startSchedule, inputs=[intime, port], outputs=startStatus)
                
            
            
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
                gr.Markdown("## "+str(datetime.now()))
                
                
                bl.load(plot_random,outputs=randomPlot)
                
            with gr.Column():
                gr.Markdown("## display prediction bar here")
                predictionBar = gr.Plot()
                predictionResult = gr.Plot()
                bl.load(plot_random,outputs=predictionBar)
                bl.load(plot_random,outputs=predictionResult)


def startGradio():
    print("Star Gradio Server")
    bl.launch(server_port=8070, debug=True)

def main():
    schedule.clear()
    while True:
        schedule.run_pending()
        dirname = os.path.join(os.path.dirname(__file__), 'csv','*') 
        listFile = glob.glob(dirname)
        lastFile = max(listFile, key=os.path.getmtime)
        print(lastFile)
        
        
        
gradioThread = threading.Thread(target=startGradio, daemon=True)
gradioThread.start()

print("Starting Main Program!!")
main()
    

    
    



