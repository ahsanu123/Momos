# -*- coding: utf-8 -*-
"""
Created on Thu Apr  6 06:33:01 2023

@author: Ahsanu amala
"""

import schedule # not built in library
import time
import os
import signal
import subprocess
from datetime import datetime
import psutil

# com = "com9"
# filename = str(datetime.today().strftime('%Y_%m_%d__%H%M%S')) + ".csv"

# command = ["cat", com]
# p = subprocess.Popen(command)

# if p.poll() is None:
#     print("task is running brow!!")
#     print(">> "+str(p))
    

# current_process = psutil.Process()
# children = current_process.children(recursive=True)
# print(len(children))
# for child in children:
#     print('Child pid is {}'.format(child.pid))

# time.sleep(5)

# p.terminate()

schedule.clear()

def getRawData(com):
    currentProcess = psutil.Process()
    children = currentProcess.children(recursive=True)
    
    # if still have another process, kill it
    if len(children)>0:
        for child in children:
            os.kill(child.pid, signal.SIGTERM)
            
    # execute operation
    filename = str(datetime.today().strftime('%Y_%m_%d__%H%M%S')) + ".csv"
    file = open(filename, "w+")
    
    command = ["cat", com]
    p = subprocess.Popen(command, stdout=file)
    print(f'{p}\t{p.pid}')

# make schedule to run task
schedule.every(5).seconds.do(getRawData,com="com9")

while True:
    schedule.run_pending()
    time.sleep(1)
    
            
            
            
            
            
            
            
            
    