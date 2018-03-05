#ifdef DOUBLE
typedef double precision;
#elif SINGLE
typedef float precision;
#else
typedef int precision;
#endif
typedef double exactPrecision;

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <cmath>
#include <limits>
#include <typeinfo>

using namespace std;

// !!! ONLY ALTER THIS CODE !!!
// ----------------------------

struct polynomial
{
    int degree = 9;
    precision coefficients[10] = {-512,2304,-4608,5376,-4032,2016,-672,144,-18,1};
    exactPrecision exact(precision xx) {
        return pow(xx-2 , 9);
    }; 
} poly;
precision leftBound = 1.91;
precision rightBound = 2.1;
int N = 501;
precision dx = (rightBound - leftBound) / (N - 1);

/*
struct polynomial
{
    int degree = 6;
    precision coefficients[7] = {15625, 18750, 9375, 2500, 375, 30, 1};
    exactPrecision exact(precision xx) {
        return pow(xx+5 , 6);
    }; 
} poly;
precision leftBound = -5.1;
precision rightBound = -4.9;
int N = 501;
precision dx = (rightBound - leftBound) / (N - 1);
*/
/*
struct polynomial
{
    int degree = 3;
    precision coefficients[4] = {-512, 192, -24, 1};
    exactPrecision exact(precision xx) {
        return pow(xx-8 , 3);
    }; 
} poly;
precision leftBound = 7.91;
precision rightBound = 8.1;
int N = 501;
precision dx = (rightBound - leftBound) / (N - 1);
*/
// !!! DO NOT ALTER CODE PASSED THIS LINE !!!
// ------------------------------------------

void summary(const char* prec) {
    printf("=========================\n");
    printf("Precision: %s\n", prec);
    printf("Polynomial: ");
    for(int i = poly.degree; i > 0; i--)
    	printf("%ix^%i ", (int)poly.coefficients[i], i);
    printf("%i\n", (int)poly.coefficients[0]);
    printf("=========================\n");
}

float HornersMethod(const precision xx) {
    precision qq = poly.coefficients[poly.degree];
    for(int i = poly.degree-1; i >= 0; i--) {
        qq = xx * qq + poly.coefficients[i];
    }
    return qq;
}

precision HornersRunningError(const precision xx) {
	
    precision qq = poly.coefficients[poly.degree];
    precision runningError = 0;
    // precision runningError = abs(qq) / 2;
    for(int i = poly.degree-1; i >= 0; i--) {
        runningError = abs(xx) * runningError + abs(xx) * abs(qq);  
        qq = xx * qq + poly.coefficients[i];
        runningError +=  abs(qq);
        //runningError = abs(xx) * runningError + abs(qq);
    }
    //runningError = numeric_limits<precision>::epsilon() * (2 * runningError - abs(qq));
    return (runningError * numeric_limits<precision>::epsilon());
}


precision ForwardError(const precision xx) {
    precision pTilde = poly.coefficients[poly.degree];
    for(int i = poly.degree-1; i >= 0; i--) {
        pTilde = abs(xx) * pTilde + abs(poly.coefficients[i]);
    }
    precision machineEps = numeric_limits<precision>::epsilon();
    precision bound = pTilde * (2 * poly.degree * machineEps) / 
                      (1 - 2 * poly.degree * machineEps);
    return bound;
}

int main(int argc, char *argv[]) {

	ofstream myfile;
	precision test;
	char fileName[14];
	if(strcmp(typeid(test).name() ,"f") == 0)
		strcpy(fileName, "dataSingle2.txt");
	else
		strcpy(fileName, "singleHornerdoubleErro.txt");
	myfile.open(fileName);

	summary(typeid(test).name());

    for(precision n = leftBound; n <= rightBound; n += dx) {
    	myfile << n << " " << poly.exact(n) << " " << HornersMethod(n) << " ";
    	myfile << ForwardError(n) << " " << poly.exact(n) + ForwardError(n);
    	myfile << " " << poly.exact(n) - ForwardError(n) << " ";
    	myfile << HornersRunningError(n) << " " << poly.exact(n) + HornersRunningError(n) << " ";
    	myfile << poly.exact(n) - HornersRunningError(n) << endl;
    }

    myfile.close();

    return 0;
}