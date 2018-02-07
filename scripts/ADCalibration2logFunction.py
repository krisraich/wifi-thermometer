# 
# ADC-Adjustment - function generator for wifi-thermometer
# Author: robsl2314 (robert@kathreins.at)
# 
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

print("Enter CSV file: ")
csvFile = input()

df = pd.read_csv(csvFile, sep=';')
print(df.T.values)
y = np.array(df.T.values[-1])
for x in df.T.values[:-1]:
	nline = np.array(x)
	reg = np.polyfit(np.log(nline), y , 1)
	reg_fn = np.poly1d(reg)
	
	fit = np.polyfit(x,y,2)
	fit_fn = np.poly1d(fit) 
	plt.plot(x,y, 'yo', x, fit_fn(x), '--k')
	plt.show()
	
	print(fit_fn)
	
	