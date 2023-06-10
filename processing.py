import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

from scipy import signal
from scipy.fft import fft, fftfreq

data = pd.read_csv('outputu70Motor2.csv')
data.head()
data.columns=['time', 'i1', 'i2', 'i3', 'vtemp','vref', 'x', 'y', 'z', 'tempa']
data.head()

time = np.array(data['time'])
x = np.array(data['i2']/4095.0*3.3)
y = np.array(data['y'])
z = np.array(data['z'])

fig, ax = plt.subplots()
plt.xlim(38000, 40000)
ax.plot(time, x, label="x")
ax.plot(time, y, label="y")
ax.plot(time, z, label="z") 
plt.legend()
plt.show()

fs = 250
f, t, z = signal.stft(x,fs)
#plt.pcolormesh(t, f, np.abs(z), cmap='gray')

yf = fft(x, norm="forward")
xf = fftfreq(len(x), 1.0/1000.0)[:len(x)//2]
plt.plot(xf, np.abs(yf[:len(x)//2]))
plt.grid()
plt.show()

