# -*- coding: utf-8 -*-
"""
Created on Sun Mar 26 04:49:08 2023

@author: Ahsanu amala
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import serial
import serial.tools.list_ports as serPorts
import threading
import glob

import plotly.express as px
import plotly.graph_objects as go
from plotly.subplots import make_subplots

import math
from scipy.fft import fft, fftfreq


import time
import schedule # not built in library
import os
import signal
import subprocess
from datetime import datetime
import psutil

import gradio as gr
gr.close_all()

# ==========================================================================
# Global Variable Section 
# ==========================================================================
latestFile = ''
# fig = go.Figure()
# fig.add_scatter(x=x, y=y, mode="lines", name ="fft")


# ==========================================================================
# Class section
# ==========================================================================

                

# ==========================================================================
# Function section
# ==========================================================================
plot_end = 2 * math.pi


def get_plot(period=1):
    global plot_end
    x = np.arange(plot_end - 2 * math.pi, plot_end, 0.02)
    y = np.sin(2*math.pi*period * x)
    fig = px.line(x=x, y=y)
    plot_end += 2 * math.pi
    if plot_end > 1000:
        plot_end = 2 * math.pi
    return fig

def currentPlot():
    global latestFile
    data = pd.read_csv(latestFile)
    data.columns = ['time', 'ia', 'ib', 'ic' ,'vref', 'vtemp', 'x', 'y', 'z', 'temp' ]
    
    ia = data['ia']
    ib = data['ib']
    ic = data['ic']
    
    ia = np.array(ia)
    ib = np.array(ib)
    ic = np.array(ic)
    ia = ia/4095.0 *3.3
    ib = ib/4095.0 *3.3
    ic = ic/4095.0 *3.3
    
    # IA =============================
    iaffty = fft(ia, norm="forward")
    iafftx = fftfreq(len(ia), 1.0/100.0)
    
    iaffty = np.abs(iaffty[0:len(iaffty)//2])
    iafftx = iafftx[0:len(iafftx)//2] 
    
    # IB =============================
    ibffty = fft(ib, norm="forward")
    ibfftx = fftfreq(len(ib), 1.0/100.0)
    
    ibffty = np.abs(ibffty[0:len(ibffty)//2])
    ibfftx = ibfftx[0:len(ibfftx)//2] 
    
    # IC =============================
    icffty = fft(ic, norm="forward")
    icfftx = fftfreq(len(ic), 1.0/100.0)
    
    icffty = np.abs(icffty[0:len(icffty)//2])
    icfftx = icfftx[0:len(icfftx)//2] 
    
    fig = make_subplots(rows=3, cols=2)
    
    
    # make figure of each signal
    figIA = go.Scatter(x=iafftx, y=iaffty, mode="lines", name="IA")
    figIB = go.Scatter(x=ibfftx, y=ibffty, mode="lines", name="IB")
    figIC = go.Scatter(x=icfftx, y=icffty, mode="lines", name="IC")
    figOriIA = go.Scatter(x=np.arange(0, len(ia)), y=ia, mode="lines", name="time IA")
    figOriIB = go.Scatter(x=np.arange(0, len(ib)), y=ib, mode="lines", name="time IB")
    figOriIC = go.Scatter(x=np.arange(0, len(ic)), y=ic, mode="lines", name="time IC")
    
    fig.append_trace(figIA, row=1, col=1)
    fig.append_trace(figIB, row=2, col=1)
    fig.append_trace(figIC, row=3, col=1)
    
    fig.append_trace(figOriIA, row=1, col=2)
    fig.append_trace(figOriIB, row=2, col=2)
    fig.append_trace(figOriIC, row=3, col=2)
    
    fig['layout']['xaxis']['title']='Freq (Hz)'
    fig['layout']['xaxis2']['title']='time (ms)'
    fig['layout']['xaxis3']['title']='Freq (Hz)'
    fig['layout']['xaxis4']['title']='time (ms)'
    fig['layout']['xaxis5']['title']='Freq (Hz)'
    fig['layout']['xaxis6']['title']='time (ms)'
    
    fig['layout']['yaxis']['title']='IA amplitude'
    fig['layout']['yaxis2']['title']='IA amplitude'
    fig['layout']['yaxis3']['title']='IB amplitude'
    fig['layout']['yaxis4']['title']='IB amplitude'
    fig['layout']['yaxis5']['title']='IC amplitude'
    fig['layout']['yaxis6']['title']='IC amplitude'
    
    fig.update_layout(autosize=True,width=1200,height=900)
    
    return fig


def vibrationPlot():
    global latestFile
    data = pd.read_csv(latestFile)
    data.columns = ['time', 'ia', 'ib', 'ic' ,'vref', 'vtemp', 'x', 'y', 'z', 'temp' ]
    
    vibx = data['x']
    viby = data['y']
    vibz = data['z']
    
    vibx = np.array(vibx)
    viby = np.array(viby)
    vibz = np.array(vibz)
    
    # VibX =============================
    vibxffty = fft(vibx, norm="forward")
    vibxfftx = fftfreq(len(vibx), 1.0/100.0)
    
    vibxffty = np.abs(vibxffty[0:len(vibxffty)//2])
    vibxfftx = vibxfftx[0:len(vibxfftx)//2] 
    
    # VibY =============================
    vibyffty = fft(viby, norm="forward")
    vibyfftx = fftfreq(len(viby), 1.0/100.0)
    
    vibyffty = np.abs(vibyffty[0:len(vibyffty)//2])
    vibyfftx = vibyfftx[0:len(vibyfftx)//2] 
    
    # VibZ =============================
    vibzffty = fft(vibz, norm="forward")
    vibzfftx = fftfreq(len(vibz), 1.0/100.0)
    
    vibzffty = np.abs(vibzffty[0:len(vibzffty)//2])
    vibzfftx = vibzfftx[0:len(vibzfftx)//2] 
    
    fig = make_subplots(rows=3, cols=2)
    
    
    # make figure of each signal
    figVibX = go.Scatter(x=vibxfftx, y=vibxffty, mode="lines", name="VibX")
    figVibY = go.Scatter(x=vibyfftx, y=vibyffty, mode="lines", name="VibY")
    figVibZ = go.Scatter(x=vibzfftx, y=vibzffty, mode="lines", name="VibZ")
    figOriVibx = go.Scatter(x=np.arange(0, len(vibx)), y=vibx, mode="lines", name="time Vibx")
    figOriViby = go.Scatter(x=np.arange(0, len(viby)), y=viby, mode="lines", name="time Viby")
    figOriVibz = go.Scatter(x=np.arange(0, len(vibz)), y=vibx, mode="lines", name="time Vibz")
    
    fig.append_trace(figVibX, row=1, col=1)
    fig.append_trace(figVibY, row=2, col=1)
    fig.append_trace(figVibZ, row=3, col=1)
    
    fig.append_trace(figOriVibx, row=1, col=2)
    fig.append_trace(figOriViby, row=2, col=2)
    fig.append_trace(figOriVibz, row=3, col=2)
    
    fig['layout']['xaxis']['title']='Freq (Hz)'
    fig['layout']['xaxis2']['title']='time (ms)'
    fig['layout']['xaxis3']['title']='Freq (Hz)'
    fig['layout']['xaxis4']['title']='time (ms)'
    fig['layout']['xaxis5']['title']='Freq (Hz)'
    fig['layout']['xaxis6']['title']='time (ms)'
    
    fig['layout']['yaxis']['title']='VibX amplitude'
    fig['layout']['yaxis2']['title']='VibX amplitude'
    fig['layout']['yaxis3']['title']='VibY amplitude'
    fig['layout']['yaxis4']['title']='VibY amplitude'
    fig['layout']['yaxis5']['title']='VibZ amplitude'
    fig['layout']['yaxis6']['title']='VibZ amplitude'
    
    fig.update_layout(autosize=True,width=1200,height=900)
    
    return fig
    

def preprocessing():
    global latestFile
    # global fig
    
    data = pd.read_csv(latestFile)
    data.columns = ['time', 'ia', 'ib', 'ic' ,'vref', 'vtemp', 'x', 'y', 'z', 'temp' ]
    
    ia = data['ia']
    ib = data['ib']
    ic = data['ic']
    vibx = data['x']
    viby = data['y']
    vibz = data['z']
    temp = data['temp']
    
    ia = np.array(ia)
    ib = np.array(ib)
    ic = np.array(ic)
    ia = ia/4095.0 *3.3
    ib = ib/4095.0 *3.3
    ic = ic/4095.0 *3.3
    
    vibx = np.array(vibx)
    viby = np.array(viby)
    vibz = np.array(vibz)
    temp = np.array(temp)
    
    """
    calculate each of fft from signal
    """
    # IA =============================
    iaffty = fft(ia, norm="forward")
    iafftx = fftfreq(len(ia), 1.0/100.0)
    
    iaffty = np.abs(iaffty[0:len(iaffty)//2])
    iafftx = iafftx[0:len(iafftx)//2] 
    
    # IB =============================
    ibffty = fft(ib, norm="forward")
    ibfftx = fftfreq(len(ib), 1.0/100.0)
    
    ibffty = np.abs(ibffty[0:len(ibffty)//2])
    ibfftx = ibfftx[0:len(ibfftx)//2] 
    
    # IC =============================
    icffty = fft(ic, norm="forward")
    icfftx = fftfreq(len(ic), 1.0/100.0)
    
    icffty = np.abs(icffty[0:len(icffty)//2])
    icfftx = icfftx[0:len(icfftx)//2] 
    
    # VibX =============================
    vibxffty = fft(vibx, norm="forward")
    vibxfftx = fftfreq(len(vibx), 1.0/100.0)
    
    vibxffty = np.abs(vibxffty[0:len(vibxffty)//2])
    vibxfftx = vibxfftx[0:len(vibxfftx)//2] 
    
    # VibY =============================
    vibyffty = fft(viby, norm="forward")
    vibyfftx = fftfreq(len(viby), 1.0/100.0)
    
    vibyffty = np.abs(vibyffty[0:len(vibyffty)//2])
    vibyfftx = vibyfftx[0:len(vibyfftx)//2] 
    
    # VibZ =============================
    vibzffty = fft(vibz, norm="forward")
    vibzfftx = fftfreq(len(vibz), 1.0/100.0)
    
    vibzffty = np.abs(vibzffty[0:len(vibzffty)//2])
    vibzfftx = vibzfftx[0:len(vibzfftx)//2] 
    
    """
    calculate Parks vector
    """
    
    parksId = np.sqrt(2./3 * ia) - 1./np.sqrt(6) * ib - 1./np.sqrt(6) * ic
    parksIq = 1./np.sqrt(2) * ib - 1./np.sqrt(2) * ic
    
    fig = make_subplots(rows=3, cols=3)
    
    
    # make figure of each signal
    figIA = go.Scatter(x=iafftx, y=iaffty, mode="lines", name="IA")
    figIB = go.Scatter(x=ibfftx, y=ibffty, mode="lines", name="IB")
    figIC = go.Scatter(x=icfftx, y=icffty, mode="lines", name="IC")
    figVibX = go.Scatter(x=vibxfftx, y=vibxffty, mode="lines", name="VibX")
    figVibY = go.Scatter(x=vibyfftx, y=vibyffty, mode="lines", name="VibY")
    figVibZ = go.Scatter(x=vibzfftx, y=vibzffty, mode="lines", name="VibZ")
    parks = go.Scatter(x=parksId, y=parksIq, mode="lines", name="Park's")
    
    fig.append_trace(figIA, row=1, col=1)
    fig.append_trace(figIB, row=1, col=2)
    fig.append_trace(figIC, row=1, col=3)
    
    fig.append_trace(figVibX, row=2, col=1)
    fig.append_trace(figVibY, row=2, col=2)
    fig.append_trace(figVibZ, row=2, col=3)
    
    fig.append_trace(parks, row=3, col=1)
    
    fig['layout']['xaxis']['title']='Freq (Hz)'
    fig['layout']['xaxis2']['title']='Freq (Hz)'
    fig['layout']['xaxis3']['title']='Freq (Hz)'
    
    fig['layout']['yaxis']['title']='IA amplitude'
    fig['layout']['yaxis2']['title']='IB amplitude'
    fig['layout']['yaxis3']['title']='IC amplitude'
    
    fig['layout']['xaxis4']['title']='Freq (Hz)'
    fig['layout']['xaxis5']['title']='Freq (Hz)'
    fig['layout']['xaxis6']['title']='Freq (Hz)'
    
    fig['layout']['yaxis4']['title']='Vibration X'
    fig['layout']['yaxis5']['title']='Vibration Y'
    fig['layout']['yaxis6']['title']='Vibration Z'
    
    fig['layout']['xaxis7']['title']='id'
    
    fig['layout']['yaxis7']['title']='iq'
    
    fig.update_layout(autosize=False,width=1200,height=900)
    
    
    return fig

def updatePreprocessing():
    global fig
    
    x = [1,2,3,4,5,6,7,8,9]
    y = np.random.rand(9)
    
    fig.data[0].x=x
    fig.data[0].y=y

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
    1. buat fungsi dengan multiprocessing untuk mengambil data dari serial ✔️
    2. buat fungsi untuk melakukan plot, preprocessing dan menampilkan data input untuk inference 
    3. buat plot untuk menampilkan hasil kemungkinan dari model
    4. buat fungsi untuk menampilkan hasil klasifikasi (sehat,BRB,dll)
    5. buat fungsi untuk setting, seperti com port baudrate dsb ✔️
    6. [optional] buat fungsi untuk menampilkan informasi model 
    7. buat fungsi untuk menampilkan hasil pembacaan dari sensor 
    8. [optional] buat fungsi untuk menampilkan history request dari modbus
    9. buat fungsi untuk mengganti seting modbus ✔
"""


# ==========================================================================
# Block user interface
# ==========================================================================

bl = gr.Blocks(title="MoMoS - Motor Monitoring system")
with bl:
    gr.Markdown("# MoMos - Motor Monitoring System Dashboard")    
    
    with gr.Tab("STATUS"):
        gr.Markdown("# Status")
    
    with gr.Tab("DASHBOARDS"):
        gr.Markdown("# Dashboard")
        
        with gr.Column():
            gr.Markdown("## Time Series ⌚ ")
            up=gr.Button(value="Update From Last Data")
            plotCur1 = gr.Plot(label="Dashboard 1")
            
            gr.Markdown("### Current")
            plotCur2 = gr.Plot(label="Current Plot")
            
            gr.Markdown("### Vibration")
            plotCur3 = gr.Plot(label="Vibration Plot")
            
            bl.load(preprocessing, None, plotCur1)
            up.click(preprocessing, None, plotCur1)
            repeat1 = bl.load(currentPlot, None, plotCur2, every=6)
            repeat2 = bl.load(vibrationPlot, None, plotCur3, every=6)
                
    
    with gr.Tab("SETTING"):
        with gr.Column():
            gr.Markdown("## Port Settings: ")
            portListMd = gr.Markdown()
            
            with gr.Row():
                port = gr.Textbox(max_lines=1, label="insert port")
                intime = gr.Number(label="Every", value=5, interactive=True)
            
            
            startStatus = gr.Markdown(value="no info")
            with gr.Row():
                listPortBtn = gr.Button(value="List Port")
                startBtn = gr.Button(value="Start")
                
                listPortBtn.click(listPort, outputs=portListMd)
                startBtn.click(startSchedule, inputs=[intime, port], outputs=startStatus)
                
            
            
            # modbus section 
            # gr.Markdown("## modbus setting: ")
            # portRadio = gr.Radio(['1', '2', '3', '4', '5'], label="Choose Modbus NUM")
            # gr.Textbox(max_lines=1, label="MODBUS NUM: ", placeholder="Choose or Insert")
            
    
def startGradio():
    print("=============================")
    print("Start Gradio Server")
    print("=============================")
    bl.queue().launch(server_port=7070, debug=True)

def main():
    global latestFile
    
    schedule.clear()
    while True:
        schedule.run_pending()
        dirname = os.path.join(os.path.dirname(__file__), 'csv','*') 
        listFile = glob.glob(dirname)
        lastFile = max(listFile, key=os.path.getmtime)
        latestFile = lastFile
        
        
        
        
gradioThread = threading.Thread(target=startGradio, daemon=True)
gradioThread.start()

print("Starting Main Program!!")
main()
# startGradio()
    

    
    



