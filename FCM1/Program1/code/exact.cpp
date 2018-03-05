#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <cmath>
#include <limits>
#include <typeinfo>

using namespace std;

float fexact1(float xx) {
	float result = pow(xx-2, 9);
	return result;
}

float fexact2(float xx) {
	float result = pow(xx+5, 6);
	return result;
}

float fexact3(float xx) {
	float result = pow(xx-8, 3);
	return result;
}

double dexact1(double xx) {
	double result = pow(xx-2,9);
	return result;
}

double dexact2(double xx) {
	double result = pow(xx+5,6);
	return result;
}

double dexact3(double xx) {
	double result = pow(xx-8,3);
	return result;
}

int main() {

	ofstream myfile;
	myfile.open("exact.txt");

	int N = 501;
	for(int i = 0; i < N; i++) {
	float fleftBound1 = 1.9;
	float frightBound1 = 2.1;
	float fleftBound2 = -5.1;
	float frightBound2 = -4.9;
	float fleftBound3 = 7.9;
	float frightBound3 = 8.1;
	double dleftBound1 = 1.9;
	double drightBound1 = 2.1;
	double dleftBound2 = -5.1;
	double drightBound2 = -4.9;
	double dleftBound3 = 7.9;
	double drightBound3 = 8.1;
	double ddx = (drightBound1 - dleftBound1) / (N - 1);
	float fdx = (frightBound1 - fleftBound1) / (N - 1);

	myfile << fleftBound1 + i*fdx << " ";
	myfile << fleftBound2 + i*fdx << " ";
	myfile << fleftBound3 + i*fdx << " ";
	myfile << abs(dexact1(dleftBound1 + i*ddx) - fexact1(fleftBound1 + i*fdx)) << " ";
	myfile << abs(dexact2(dleftBound2 + i*ddx) - fexact2(fleftBound2 + i*fdx)) << " ";
	myfile << abs(dexact3(dleftBound3 + i*ddx) - fexact3(fleftBound3+ i*fdx)) << endl;
	}

	myfile.close();

	return 0;
}