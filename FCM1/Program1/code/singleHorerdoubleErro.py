import numpy as np
import matplotlib.pyplot as plt

with open('dataDouble.txt', 'r') as file:
	lines = file.readlines()
file.close()

x = [];
x1 = [];
horner = [];
exact = [];
forwardUpper = [];
forwardLower = [];
runningUpper = [];
runningLower = [];

for line in lines:
	x.append(line.split()[0])
	exact.append(line.split()[1])
	forwardUpper.append(line.split()[4])
	forwardLower.append(line.split()[5])
	runningUpper.append(line.split()[7])
	runningLower.append(line.split()[8])

with open('dataSingle.txt', 'r') as file:
	lines = file.readlines()
file.close()

for line in lines:
	x1.append(line.split()[0])
	horner.append(line.split()[2])
#----------------------------------------------------------------------
#----------------------------------------------------------------------

plt.plot(x, exact, label='exact', c='k', linewidth=2.5)
plt.plot(x1, horner, label='Horner (Single)', c='r', linewidth=0.5)
plt.plot(x, forwardLower, label='Forward Error (Double)', c='g', linewidth=2.5)
plt.plot(x, forwardUpper, c='g', linewidth=2.5)
plt.plot(x, runningLower, label='Running Error (Double)', c='b', linewidth=2.5)
plt.plot(x, runningUpper, c='b', linewidth=2.5)
plt.xlim([1.91, 2.1])
plt.ylim([-2E-9, 2E-9])
leg =plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.025),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)
plt.title('Horner with Double Precision Error Bounds', fontsize=18)
plt.xlabel('$x$', fontsize=16)
plt.ylabel('$p_n(x)$', fontsize=16)
plt.savefig('singleHornerdoubleErro.png')
#plt.show()