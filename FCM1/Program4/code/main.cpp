#ifdef DOUBLE
typedef double precision;
#else
typedef float precision;
#endif

/* Makes sure refinement process does not go on too long */
#define MAX_REFINEMENT 50
/* Number of digits to print out */
#define MAX_DIGITS 12
/* Function Number */
#define FUNC_NUM 1
/* Number of refinements */
#define REFINE_NUM 5

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

/* ============= FUNCTIONS TO INTEGRATE ============= */

/* FUNCTION 1: f(x) = e^x */
precision func1(precision xx) {
    precision result; result = exp(xx); return result;
}

/* FUNCTION 2: f(x) = e^(sin(2x))cos(2x) */
precision func2(precision xx) {
    precision result; result = exp(sin(10*M_PI*xx)); return result;
}

/* FUNCTION 3: f(x) = tanh(x) */
precision func3(precision xx) {
    precision result; result = tanh(xx); return result;
}

/* FUNCTION 4: f(x) = xcos(2pix) */
precision func4(precision xx) {
    precision result; result = xx * cos(2*M_PI*xx); return result;
}

/* FUNCTION 5: f(x) = x + 1/x */
precision func5(precision xx) {
    precision result; result = xx + 1 / xx; return result;
}

/* FUNCTION 6: f(x) = x + 1/x 
precision func6(precision xx) {
    precision result; result = ; return result;
}
*/

precision exact[6] = {19.08553692318766774092852965458171789698790783855415014437, 
					   1.266065877752,
					  -0.89122191687483724391125651192944945332781581115522281812,
					  -0.050660591821168885721939731604863819452179387336123,
					  6.3388758248682007492015186664523752790512027085370354,
					  -0.086413725487291025097870467093211};

void printTriangle(vector <vector<precision> > &v) {

	cout << endl << "================= ROMBERG TRIANLGE ====================" << endl << endl;

	for(int i = v[0].size()-1; i >= 0; i--) {
		for(int j = 0; j < v[0].size()-i; j++){
			cout << v[j][i] << " ";
		}
		cout << endl << endl;
	}

	cout << endl << "============= ROMBERG TRIANLGE (ERROR) ================" << endl << endl;

	for(int i = v[0].size()-1; i >= 0; i--) {
		for(int j = 0; j < v[0].size()-i; j++){
			cout << abs(v[j][i] - exact[FUNC_NUM-1]) << " ";
		}
		cout << endl << endl;
	}

	return;

} 			

/* TRAPEZOIDAL QUADRATURE */
void trapezoidalRule(vector<precision> mesh, precision maxStep, precision (*func)(precision), vector<precision> &resultVector) {

    precision result = 0;                                               /* Store result */
    precision h = mesh[1] - mesh[0];                                    /* Step size */
    int refinement = 0;                                                 /* Track refinement iterations */
    precision error = 0;                                                /* Error in quadrature */
	vector<precision> resultVecotr;										/* Stores results */

    /* Compute trapezoidal rule on given mesh */
    result = 0.5 * ((*func)(mesh[0]) + (*func)(mesh[mesh.size()-1]));
    for(int i = 1; i < mesh.size()-1; i++)
        result = result + (*func)(mesh[i]);

    /* Push non-refined quadrature */
    resultVector.push_back(result * h);

    /* Apply mesh refinement until we reach step size tolerance. This is triggered when 
       minStepSize is set to -1. */
    while((maxStep != -1) &&  (refinement < maxStep)) {

        /* Update step size */
        h = h / 2;
        
        /* Compute new trapezoidal rule evaluation */
        for(int i = 1; i < pow(2, refinement) * (mesh.size()-1) + 1; i++) 
            result = result + (*func)(mesh[0] + (2*i - 1) * h);

        /* Push back refined quadrature */
        resultVector.push_back(result * h);

        /* Update refiniemt number */
        refinement++;

        /* Exit refinement if we are at max iterations */
        if(refinement > MAX_REFINEMENT) {
            cout << "WARNING: Max refinement reached, exiting method." << endl;
            break;
        }

    }

    return;

}

/* ROMBERG METHOD */
void rombergMethod(vector<precision> mesh, precision maxStep, precision (*func)(precision), vector<precision> &result) {

	int size = maxStep * maxStep;
	vector< vector<precision> > triangle;

	/* Check if beginning mesh size is 2 so we can apply Romberg */
	if(mesh.size() != 2) {
		cout << "ERROR: Please start with mesh of size 2 (end points of domain)" << endl;
		return;
	}

	/* Get first column of Romberg triangle */
	trapezoidalRule(mesh, maxStep, func, result);
	vector<precision> data;

	cout << "================== REFINEMENT VALUES ===================" << endl << endl;
	for(int i = 0; i <= maxStep; i++) {
		cout << result[i] << " & " << abs(result[i]-exact[FUNC_NUM-1]) << " & " << endl;
		data.push_back(result[i]);
	}
	reverse(data.begin(), data.end());
	triangle.push_back(data);
	data.clear();
	cout << endl;

	/* Compute and store the diagonal of the Romberg triangle */
	for(int i = 1; i <= maxStep; i++) {
		for(int j = maxStep; j >= i; j--) {
			result[j] = (pow(4, i) * result[j] - result[j-1]) / (pow(4,i) - 1);
			data.push_back(result[j]);
		}
		triangle.push_back(data);
		data.clear();
	}

	cout << "=================== ROMBERG RESULTS ===================" << endl << endl;
	for(int i = 0; i <= maxStep; i++)
		cout << result[i] << " & " << abs(result[i] - exact[FUNC_NUM-1]) << endl;
	cout << endl;

	printTriangle(triangle);

	return;

}

/* MAIN FUNCTION */
int main(int argc, char *argv[]) {

	ofstream myfile;	
	myfile.open("data.txt");

	cout << std::setprecision(MAX_DIGITS) << fixed;
	
	vector<precision> trapResults;
	vector<precision> testResults;
	vector<precision> xPoints;

	xPoints.push_back(0.0);
	xPoints.push_back(3.0);

	cout << endl << "======================== SUMMARY =======================" << endl << endl;
	cout << "Left bound " << xPoints[0] << endl;
	cout << "Right Bound " << xPoints[1] << endl;
	cout << "Function Number : " << FUNC_NUM << endl << endl;

	trapezoidalRule(xPoints, REFINE_NUM, func1, trapResults);
	rombergMethod(xPoints, REFINE_NUM, func1, testResults);

	for(int i = 0; i <= REFINE_NUM; i++) {
		myfile << i+1 << " "; 
		myfile << -log10(abs(exact[FUNC_NUM-1] - trapResults[i])) << " ";
		myfile << -log10(abs(exact[FUNC_NUM-1] - testResults[i])) << endl;
	}

	myfile.close();

	return 0;

}