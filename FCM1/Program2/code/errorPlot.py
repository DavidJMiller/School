import numpy as np
import matplotlib.pyplot as plt

with open('error.txt', 'r') as file:
	lines = file.readlines()
file.close()

x = [];
NM = [];
BM = [];
PW = [];
CS = [];

for line in lines: 
	x.append(line.split()[0])
	NM.append(line.split()[1])
	BM.append(line.split()[2])
	PW.append(line.split()[3])
	CS.append(line.split()[4])

plt.figure()
plt.plot(x, NM, label="Newton Method", c='r')
plt.plot(x, BM, label='Berstein Method', c='b')
plt.plot(x, PW, label='Piecewise Method', c='g')
plt.plot(x, CS, label='Cubic Spline', c='y')
plt.title('Error (Single Precision)', fontsize=20)
plt.xlabel('$x$', fontsize=20)
plt.ylabel('$| f(x) - f_{approx}(x) |$', fontsize=20)
leg =plt.legend(loc='lower center', bbox_to_anchor=(0.5, 0.675),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)
plt.savefig('NerrorFig4.eps')

plt.figure()
plt.plot(x, NM, label="Newton Method", c='r')
plt.plot(x, BM, label='Berstein Method', c='b')
plt.plot(x, PW, label='Piecewise Method', c='g')
plt.plot(x, CS, label='Cubic Spline', c='y')
plt.title('Error (Zoomed In)', fontsize=20)
plt.xlabel('$x$', fontsize=20)
plt.ylabel('$| f(x) - f_{approx}(x) |$', fontsize=20)
leg =plt.legend(loc='lower center', bbox_to_anchor=(0.5, 0.675),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)
plt.ylim(0,0.01)
plt.savefig('NerrorFigg4.eps')

#plt.show()