import numpy as np
import matplotlib.pyplot as plt

with open('mesh.txt', 'r') as file:
	lines = file.readlines()
file.close()

x = [];
y = [];
m1 = [];
m2 = [];
m3 = [];
m4 = [];
m5 = [];
m6 = [];

for line in lines: 
	x.append(line.split()[0])
	y.append(line.split()[1])
	m1.append(line.split()[2])
	m2.append(line.split()[3])
	m3.append(line.split()[4])
	m4.append(line.split()[5])
	m5.append(line.split()[6])
	m6.append(line.split()[7])

plt.figure()
plt.plot(x, y, label='Exact', c='k')
plt.plot(x, m1, label='5 Mesh Points', c ='b')
plt.plot(x, m2, label='15 Mesh Points', c ='y')
plt.plot(x, m3, label='25 Mesh Points', c ='r')
plt.plot(x, m4, label='35 Mesh Points', c ='g')
plt.plot(x, m5, label='45 Mesh Points', c ='m')
plt.plot(x, m6, label='55 Mesh Points', c ='c')
leg =plt.legend(loc='lower center', bbox_to_anchor=(0.5, 0.025),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)
plt.title("Bernstein Polynomials (Double Precision)", fontsize=20)
plt.xlabel("$x$", fontsize=20)
plt.ylabel('$f_{approx}(x)$', fontsize=20)
plt.savefig('bernSingle.eps')
#plt.show()