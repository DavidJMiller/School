import numpy as np
import matplotlib.pyplot as plt
from math import log

with open('data.txt', 'r') as file:
	lines = file.readlines()
file.close()	

x = [];
eTrap = [];
eRom = [];

for line in lines: 
	x.append(line.split()[0])
	eTrap.append(line.split()[1])
	eRom.append(line.split()[2])

plt.figure()
plt.plot(x, eTrap, label="Trapezoidal", c='r')
plt.plot(x, eRom, label='Romberg', c='g')
plt.title('Error Convergence', fontsize=20)
plt.xlabel('$k$', fontsize=20)
plt.ylabel('$-log(|error|)$', fontsize=20)
leg =plt.legend(loc='lower center', bbox_to_anchor=(0.2, 0.775),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)
#plt.show()
plt.savefig('convergence6.eps')