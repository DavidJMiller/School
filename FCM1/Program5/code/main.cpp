#ifdef DOUBLE
typedef double precision;
#else
typedef float precision;
#endif

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <algorithm>
#include <string.h>
#include <vector>
#include <cmath>
#include <limits>
#include <typeinfo>
#include <utility>

using namespace std;

precision INIT_COND = 1.0;
precision LAMBDA = -500;
precision LEFT_BOUND = 0.0;
precision RIGHT_BOUND = 1.0;
precision H = 0.001;

/* Exact Solution to IBVP */
precision exact(precision tt) {
	return INIT_COND * exp(LAMBDA * tt)	+ sin(tt);
}

/* RHS of model problem (y' = f) */
precision f(precision tt, precision yy) {
	return LAMBDA * (yy - sin(tt)) + cos(tt);
}

/* First method to approximate the ODE solution */
void method1(vector<precision>	&yy) {

	/* Push back initial condition */
	yy.push_back(INIT_COND);

	/* Push back exact solution at tt = H */
	yy.push_back(exact(LEFT_BOUND + H));

	/* For loop that executes the method to compute solution */
	precision y_n = 0;
	for(int i = 2; i <= (RIGHT_BOUND - LEFT_BOUND) / H; i++) {
		y_n = -4 * yy[i-1] + 5 * yy[i-2] + H * (4 * f(LEFT_BOUND + (i - 1) * H, yy[i-1]) + 2 * f(LEFT_BOUND + (i - 2) * H, yy[i-2]));
		yy.push_back(y_n);
	}

	return;

}


/* Third method to approximate the ODE solution */
void method2(vector<precision> &yy) {

	/* Push back initial condition */
	yy.push_back(INIT_COND);

	/* Push back exact solution at tt = H */
	yy.push_back(exact(LEFT_BOUND + H));

		/* For loop that executes the method to compute solution */
	precision y_n = 0;
	for(int i = 2; i <= (RIGHT_BOUND - LEFT_BOUND) / H; i++) {
		y_n = yy[i-2] + 2 * H * f(LEFT_BOUND + (i - 1) * H, yy[i-1]);
		yy.push_back(y_n);
	}

	return;

}

/* Second method to approximate the ODE solution */
void method3(vector<precision> &yy) {

	/* Push back initial condition */
	yy.push_back(INIT_COND);

	/* Push back exact solution at tt = H */
	yy.push_back(exact(LEFT_BOUND + H));

		/* For loop that executes the method to compute solution */
	precision y_n = 0;
	for(int i = 2; i <= (RIGHT_BOUND - LEFT_BOUND) / H; i++) {
		y_n = yy[i-1] + (H / 2) * (3 * f(LEFT_BOUND + (i - 1) * H, yy[i-1]) - f(LEFT_BOUND + (i-2) * H, yy[i-2])); 
		yy.push_back(y_n);
	}

	return;

}

/* Fourth method to approximate the ODE solution */
void method4(vector<precision> &yy) {

	/* Push back initial condition */
	yy.push_back(INIT_COND);

	/* For loop that executes the method to compute solution */
	precision y_n = 0;
	for(int i = 1; i <= (RIGHT_BOUND - LEFT_BOUND) / H; i++) {
		y_n = (1 / (1 - (H * LAMBDA)/ 2)) * (yy[i-1] - (H / 2) * (LAMBDA * sin(LEFT_BOUND + i * H) - cos(LEFT_BOUND + i * H) - f(LEFT_BOUND + (i-1) * H, yy[i-1]))); 
		yy.push_back(y_n);
	}

	return;

}

/* Fifth method to approximate the ODE solution */
void method5(vector<precision> &yy) {

	yy.push_back(INIT_COND);

	precision y_n = 0;
	for(int i = 1; i <= (RIGHT_BOUND - LEFT_BOUND) / H; i++) {
		y_n = (1 / (1 - H * LAMBDA)) * (yy[i-1] - H * LAMBDA * sin(LEFT_BOUND + i * H) + H * cos(LEFT_BOUND + i * H));
		yy.push_back(y_n);
	}

	return;

}

/* Sixth method to approximate the ODE solution */
void method6(vector<precision> &yy) {

	/* Push back initial condition */
	yy.push_back(INIT_COND);

	/* Push back exact solution at tt = H */
	yy.push_back(exact(LEFT_BOUND + H));

		/* For loop that executes the method to compute solution */
	precision y_n = 0;
	for(int i = 2; i <= (RIGHT_BOUND - LEFT_BOUND) / H; i++) {
		y_n = (1 / (3 - 2 * H * LAMBDA)) * (4 * yy[i-1] - yy[i-2] + 2 * H * (LAMBDA * -sin(LEFT_BOUND + i * H) + cos(LEFT_BOUND + i * H)));
		yy.push_back(y_n);
	}

	return;

}

/* MAIN FUNCTION */
int main(int argc, char *argv[]) {

	ofstream myfile;	
	myfile.open("data.txt");

	vector<precision> results1;
	vector<precision> results2;
	vector<precision> results3;
	vector<precision> results4;
	vector<precision> results5;
	vector<precision> results6;
	method1(results1);
	method2(results2);
	method3(results3);
	method4(results4);
	method5(results5);
	method6(results6);

	for(int i = 0; i <= (RIGHT_BOUND - LEFT_BOUND) / H; i++) {
		myfile << LEFT_BOUND + i * H << " ";
		myfile << exact(LEFT_BOUND + i * H) << " ";
		myfile << results1[i] << " ";
		myfile << results2[i] << " ";
		myfile << results3[i] << " ";
		myfile << results4[i] << " ";
		myfile << results5[i] << " ";
		myfile << results6[i] << endl;
	}

	myfile.close();

	return 0;

}