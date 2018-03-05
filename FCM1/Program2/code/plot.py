import numpy as np
import matplotlib.pyplot as plt

with open('data.txt', 'r') as file:
	lines = file.readlines()
file.close()

x = [];
xNewton = [];
xcNewton = [];
'''
BM = [];
PW = [];
CS = [];
'''

for line in lines:
	x.append(line.split()[0])
	xNewton.append(line.split()[1])
	xcNewton.append(line.split()[2])

'''
plt.plot(x, y, label='Exact', c='k')
plt.plot(x, NM, label='Newton Method', c='r')
plt.plot(x, BM, label='Bernstein Method', c='b')
plt.plot(x, PW, label='Piecewise Method (Quadratic)', c='g')
plt.plot(x, CS, label='Cubic Spline (Natural)', c='y')

leg =plt.legend(loc='lower center', bbox_to_anchor=(0.5, 0.025),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)

plt.title('All Methods', fontsize=20)
plt.xlabel('$x$', fontsize=20)
plt.ylabel('$y(x)$', fontsize=20)
plt.savefig('Nfig1.eps')

plt.figure()
plt.subplot(2,1,1)
plt.plot(x[200:300], y[200:300], label='Exact', c='k')
plt.plot(x[200:300], NM[200:300], label='Newton Method', c='r')
plt.plot(x[200:300], BM[200:300], label='Bernstein Method', c='b')
plt.plot(x[200:300], PW[200:300], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[200:300], CS[200:300], label='Cubic Spline', c='y')
plt.title('Peak (Zoomed In)')
plt.xlim(-0.02, 0.02)
plt.ylim(0.995,1)

plt.subplot(2,2,3)
plt.plot(x[0:100], y[0:100], label='Exact', c='k')
plt.plot(x[0:100], NM[0:100], label='Newton Method', c='r')
plt.plot(x[0:100], BM[0:100], label='Bernstein Method', c='b')
plt.plot(x[0:100], PW[0:100], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[0:100], CS[0:100], label='Cubic Spline', c='y')
plt.title('Left Boundary (Zoomed In)')
plt.ylim(0.0905, 0.093	)
plt.xlim(-1, -.99)

plt.subplot(2,2,4)
plt.plot(x[400:501], y[400:501], label='Exact', c='k')
plt.plot(x[400:501], NM[400:501], label='Newton Method', c='r')
plt.plot(x[400:501], BM[400:501], label='Bernstein Method', c='b')
plt.plot(x[400:501], PW[400:501], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[400:501], CS[400:501], label='Cubic Spline', c='y')
plt.title('Right Boundary (Zoomed In)')
plt.ylim(0.0905, 0.093	)
plt.xlim(0.99, 1)
plt.savefig('NsubFig1.eps')
x#plt.show()
'''

'''
plt.plot(x, y, label='Exact', c='k')
plt.plot(x, NM, label='Newton Method', c='r')
plt.plot(x, BM, label='Bernstein Method', c='b')
plt.plot(x, PW, label='Piecewise Method (Quadratic)', c='g')
plt.plot(x, CS, label='Cubic Spline (Natural)', c='y')

leg =plt.legend(loc='lower center', bbox_to_anchor=(0.5, 0.1),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)

plt.title('All Methods (Zoomed In)', fontsize=20)
plt.xlabel('$x$', fontsize=20)
plt.ylabel('$y(x)$', fontsize=20)
plt.savefig('Nfig2.eps')

plt.figure()
plt.subplot(2,1,1)
plt.plot(x[200:300], y[200:300], label='Exact', c='k')
plt.plot(x[200:300], NM[200:300], label='Newton Method', c='r')
plt.plot(x[200:300], BM[200:300], label='Bernstein Method', c='b')
plt.plot(x[200:300], PW[200:300], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[200:300], CS[200:300], label='Cubic Spline', c='y')
plt.title('Center (Zoomed In)')
plt.xlim(-0.1, 0.1)
plt.ylim(-0.05,0.2)

plt.subplot(2,2,3)
plt.plot(x[0:100], y[0:100], label='Exact', c='k')
plt.plot(x[0:100], NM[0:100], label='Newton Method', c='r')
plt.plot(x[0:100], BM[0:100], label='Bernstein Method', c='b')
plt.plot(x[0:100], PW[0:100], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[0:100], CS[0:100], label='Cubic Spline', c='y')
plt.title('Left Boundary (Zoomed In)')
plt.ylim(-0.6, 0)
plt.xlim(-1, -.75)
plt.subplot(2,2,4)
plt.plot(x[400:501], y[400:501], label='Exact', c='k')
plt.plot(x[400:501], NM[400:501], label='Newton Method', c='r')
plt.plot(x[400:501], BM[400:501], label='Bernstein Method', c='b')
plt.plot(x[400:501], PW[400:501], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[400:501], CS[400:501], label='Cubic Spline', c='y')
plt.title('Right Boundary (Zoomed In)')
plt.ylim(0.4, 1)
plt.xlim(0.75, 1)
plt.savefig('NsubFig2.eps')
#plt.show()
'''

plt.figure()
plt.plot(x, xNewton, label='Newton (Uniform)', c='k')
plt.plot(x, xcNewton, label='Newton (Chebyshev)', c='r')
'''
plt.plot(x, BM, label='Bernstein Method', c='b')
plt.plot(x, PW, label='Piecewise Method (Quadratic)', c='g')
plt.plot(x, CS, label='Cubic Spline (Natural)', c='y')
'''

leg =plt.legend(loc='lower center', bbox_to_anchor=(0.5, 0.8),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)

plt.show()

'''

plt.title('All Methods', fontsize=20)
plt.xlabel('$x$', fontsize=20)
plt.ylabel('$y(x)$', fontsize=20)
plt.savefig('Nfig4.eps')

plt.figure()
plt.subplot(2,1,1)
plt.plot(x[200:300], y[200:300], label='Exact', c='k')
plt.plot(x[200:300], NM[200:300], label='Newton Method', c='r')
plt.plot(x[200:300], BM[200:300], label='Bernstein Method', c='b')
plt.plot(x[200:300], PW[200:300], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[200:300], CS[200:300], label='Cubic Spline', c='y')
plt.title('Center (Zoomed In)')
plt.xlim(-0.00001, 0.00001)
plt.ylim(-0.00001,0.00001)

plt.subplot(2,2,3)
plt.plot(x[0:100], y[0:100], label='Exact', c='k')
plt.plot(x[0:100], NM[0:100], label='Newton Method', c='r')
plt.plot(x[0:100], BM[0:100], label='Bernstein Method', c='b')
plt.plot(x[0:100], PW[0:100], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[0:100], CS[0:100], label='Cubic Spline', c='y')
plt.title('Left Boundary (Zoomed In)')
plt.ylim(0.4, 0.55)
plt.xlim(-10, -9.9)
plt.subplot(2,2,4)
plt.plot(x[400:501], y[400:501], label='Exact', c='k')
plt.plot(x[400:501], NM[400:501], label='Newton Method', c='r')
plt.plot(x[400:501], BM[400:501], label='Bernstein Method', c='b')
plt.plot(x[400:501], PW[400:501], label='Piecewise Method (Quadratic)', c='g')
plt.plot(x[400:501], CS[400:501], label='Cubic Spline', c='y')
plt.title('Right Boundary (Zoomed In)')
plt.ylim(-0.55, -0.4)
plt.xlim(9.9, 10)
plt.savefig('NsubFig4.eps')
#plt.show()
'''