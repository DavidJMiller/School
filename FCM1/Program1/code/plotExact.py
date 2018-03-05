import numpy as np
import matplotlib.pyplot as plt

with open('exact.txt', 'r') as file:
	lines = file.readlines()
file.close()

x1 = [];
x2 = [];
x3 = [];
dif1 = [];
dif2 = [];
dif3 = [];

for line in lines:
	x1.append(line.split()[0])
	x2.append(line.split()[1])
	x3.append(line.split()[2])
	dif1.append(line.split()[3])
	dif2.append(line.split()[4])
	dif3.append(line.split()[5])
#----------------------------------------------------------------------
#----------------------------------------------------------------------

plt.plot(x1, dif1)
plt.xlim([1.91, 2.1])
plt.title('Difference of $fl_S(p(x))$ and $fl_D(p(x))$', fontsize=20)
plt.ylabel('$|fl_D(p(x)) - fl_S(p(x))|$', fontsize=16)
plt.xlabel('$x$', fontsize=16)
plt.savefig('dif1.png')

plt.figure()
plt.plot(x2, dif2)
plt.xlim([-5.09, -4.9])
plt.title('Difference of $fl_S(p(x))$ and $fl_D(p(x))$', fontsize=20)
plt.ylabel('$|fl_D(p(x)) - fl_S(p(x))|$', fontsize=16)
plt.xlabel('$x$', fontsize=16)
plt.savefig('dif2.png')

plt.figure()
plt.plot(x3, dif3)
plt.xlim([7.91, 8.1])
plt.title('Difference bof $fl_S(p(x))$ and $fl_D(p(x))$', fontsize=20)
plt.ylabel('$|fl_D(p(x)) - fl_S(p(x))|$', fontsize=16)
plt.xlabel('$x$', fontsize=16)
plt.savefig('dif3.png')
#plt.show()