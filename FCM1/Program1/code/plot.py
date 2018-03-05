import numpy as np
import matplotlib.pyplot as plt

with open('dataSingle2.txt', 'r') as file:
	lines = file.readlines()
file.close()

x = [];
yExact = [];
yHorner = [];
yForwardError = [];
yForwardErrorUpper = [];
yForwardErrorLower = [];
yRunningError = [];
yRunningErrorUpper = [];
yRunningErrorLower = [];

for line in lines:
	x.append(line.split()[0])
	yExact.append(line.split()[1])
	yHorner.append(line.split()[2])
	yForwardError.append(line.split()[3])
	yForwardErrorUpper.append(line.split()[4])
	yForwardErrorLower.append(line.split()[5])
	yRunningError.append(line.split()[6])
	yRunningErrorUpper.append(line.split()[7])
	yRunningErrorLower.append(line.split()[8])
plt.plot(x,yExact, label="Exact (Double)", c = 'k', linewidth=1.5)
plt.plot(x, yHorner, label="Horner's Method (Single)", c = 'r', linewidth=1.5)
plt.plot(x, yForwardErrorUpper, label="Forward Error Bound (Single)", c='g', linewidth=1.5)
plt.plot(x, yForwardErrorLower, c = 'g', linewidth=1.5)
plt.plot(x, yRunningErrorUpper, label="Running Error Bound (Single)", c='b', linewidth=1.5)
plt.plot(x, yRunningErrorLower, c = 'b', linewidth=1.5)
#plt.xlim([1.91, 2.1])
#plt.xlim([7.91,8.1])
plt.xlim([-5.09,-4.9])
plt.ylabel("$p_n(x)$", fontsize=24)
plt.xlabel("$x$", fontsize=24)
leg =plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.025),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)
plt.title("Horner's Method", fontsize=20)
plt.savefig('Single3.png')

plt.figure()
plt.subplot(2,1,1)
X_detail = []
Y_exact = []
Y_horner = []
Y_runningErrorUpper = []
Y_runningErrorLower = []
for i in range(100,351):
	X_detail.append(x[i])
	Y_horner.append(yHorner[i])
	Y_exact.append(yExact[i])
	Y_runningErrorUpper.append(yRunningErrorUpper[i])
	Y_runningErrorLower.append(yRunningErrorLower[i])
plt.plot(X_detail, Y_horner, c = 'r', linewidth=1.5, label="Horner's Method (Single)") 
plt.plot(X_detail, Y_exact, c = 'k', linewidth=1.5, label="Exact (Double)")
plt.plot(X_detail, Y_runningErrorUpper, c = 'b', linewidth=1.5, label="Running Error Bound (Single)")
plt.plot(X_detail, Y_runningErrorLower, c = 'b', linewidth=1.5)
plt.xlim([-5.05,-4.96])
#plt.xlim([7.95,8.04])
#plt.xlim([1.95,2.04]) 
#plt.ylim([-0.04, 0.04])
#plt.ylim([-0.75E-10, 0.75E-10])
plt.title("Zoomed In", fontsize=20)
leg =plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.05),
          fancybox=True, shadow=True, ncol=1, fontsize=14)
for legobj in leg.legendHandles:
    legobj.set_linewidth(2.5)

plt.subplot(2,2,3)
X_left = []
Y_exactLeft = []
Y_hornerLeft = []
Y_runningErrorUpperLeft = []
Y_runningErrorLowerLeft = []
for i in range(0,100):
	X_left.append(x[i])
	Y_hornerLeft.append(yHorner[i])
	Y_exactLeft.append(yExact[i])
	Y_runningErrorUpperLeft.append(yRunningErrorUpper[i])
	Y_runningErrorLowerLeft.append(yRunningErrorLower[i])
plt.plot(X_left, Y_hornerLeft, c = 'r', linewidth=1.5) 
plt.plot(X_left, Y_exactLeft, c = 'k', linewidth=1.5)
plt.plot(X_left, Y_runningErrorUpperLeft, c = 'b', linewidth=1.5)
plt.plot(X_left, Y_runningErrorLowerLeft, c = 'b', linewidth=1.5)
#plt.xlim([1.91,1.945]) 
plt.xlim([-5.09,-5.065])
#plt.xlim([7.91,7.945])
#plt.ylim([-0.5E-9, 0.5E-9])
#plt.ylim([-1E-7, 9E-7])
#plt.xticks([1.91, 1.9275 ,1.945],[1.91, 1.9275 ,1.945])
plt.xticks([-5.09, -5.0775 ,-5.065],[-5.09, -5.0725 ,-5.055])
plt.title("Left Boundary", fontsize=16)

plt.subplot(2,2,4)
X_left = []
Y_exactLeft = []
Y_hornerLeft = []
Y_runningErrorUpperLeft = []
Y_runningErrorLowerLeft = []
for i in range(400,500):
	X_left.append(x[i])
	Y_hornerLeft.append(yHorner[i])
	Y_exactLeft.append(yExact[i])
	Y_runningErrorUpperLeft.append(yRunningErrorUpper[i])
	Y_runningErrorLowerLeft.append(yRunningErrorLower[i])
plt.plot(X_left, Y_hornerLeft, c = 'r', linewidth=1.5) 
plt.plot(X_left, Y_exactLeft, c = 'k', linewidth=1.5)
plt.plot(X_left, Y_runningErrorUpperLeft, c = 'b', linewidth=1.5)
plt.plot(X_left, Y_runningErrorLowerLeft, c = 'b', linewidth=1.5)
plt.xlim([-4.935,-4.9])
#plt.ylim([-1E-7, 9E-7])
#plt.xlim([2.065,2.1]) 
#plt.xlim([8.065,8.1])
#plt.ylim([-1E-10, 4E-10])
plt.xticks([-4.935, -4.9175, -4.9],[-4.935, -4.9175, -4.9])
#plt.xticks([2.065, 2.0825 ,2.1],[2.065, 2.0825 ,2.1])
plt.title("Right Boundary", fontsize=16)
plt.savefig('Single4.png')
#plt.show()