import numpy as np
import matplotlib.pyplot as plt
from math import log

with open('data.txt', 'r') as file:
	lines = file.readlines()
file.close()	

x = [];
exact = [];
numeric1 = [];
numeric2 = [];
numeric3 = [];
numeric4 = [];
numeric5 = [];
numeric6 = [];

for line in lines: 
	x.append(line.split()[0])
	exact.append(line.split()[1])
	numeric1.append(line.split()[2])
	numeric2.append(line.split()[3])
	numeric3.append(line.split()[4])
	numeric4.append(line.split()[5])
	numeric5.append(line.split()[6])
	numeric6.append(line.split()[7])

plt.figure()
plt.plot(x, exact, label="Exact Solution", c='k')
#plt.plot(x, numeric1, label='Method 1', c='g')
#plt.plot(x, numeric2, label='Method 2', c='b')
plt.plot(x, numeric3, label='Method 3', c='y')
plt.plot(x, numeric4, label='Method 4', c='r')
plt.plot(x, numeric5, label='Method 5', c='c')
plt.plot(x, numeric6, label='Method 6', c='m')
plt.title('Numerical vs Analytical', fontsize=20)
plt.xlabel('x', fontsize=20)
plt.ylabel('y(x)', fontsize=20)
leg =plt.legend(loc='lower center', bbox_to_anchor=(0.2, 0.575),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)
plt.xlim(0,1);	
plt.savefig('reg_-500_001(1).png');
#plt.show()