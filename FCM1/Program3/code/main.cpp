#ifdef DOUBLE
typedef double precision;
#else
typedef float precision;
#endif

/* Makes sure refinement process does not go on too long */
#define MAX_REFINEMENT 50

#include <iostream>
#include <fstream>
#include <stdio.h>
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
    precision result; result = exp(sin(2*xx)) * cos(2*xx); return result;
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

/* =============== QUADRATURE METHODS =============== */

/* TRAPEZOIDAL QUADRATURE */
precision trapezoidalRule(vector<precision> mesh, precision maxStep, precision (*func)(precision)) {

    precision result = 0;                                               /* Store result */
    precision h = mesh[1] - mesh[0];                                    /* Step size */
    int refinement = 0;                                                 /* Track refinement iterations */
    precision error = 0;                                                /* Error in quadrature */

    /* Compute trapezoidal rule on given mesh */
    result = 0.5 * ((*func)(mesh[0]) + (*func)(mesh[mesh.size()-1]));
    for(int i = 1; i < mesh.size()-1; i++)
        result = result + (*func)(mesh[i]);

    /* Apply mesh refinement until we reach step size tolerance. This is triggered when 
       minStepSize is set to -1. */
    while((maxStep != -1) &&  (refinement < maxStep)) {

        /* Update step size */
        h = h / 2;
        
        /* Compute new trapezoidal rule evaluation */
        for(int i = 1; i < pow(2, refinement) * (mesh.size()-1) + 1; i++) 
            result = result + (*func)(mesh[0] + (2*i - 1) * h);

        /* Update refiniemt number */
        refinement++;

        /* Exit refinement if we are at max iterations */
        if(refinement > MAX_REFINEMENT) {
            cout << "WARNING: Max refinement reached, exiting method." << endl;
            break;
        }

    }

    result = result * h;
    return result;

}

/* SIMPSON'S FIRST RULE */
precision simpsonRule(vector<precision> mesh, precision maxStep, precision (*func)(precision)) {

    precision result = 0;                                           /* Store result */
    precision h = mesh[1] - mesh[0];                                /* Step size */
    int refinement = 0;                                             /* Track refinement iterations */

    /* Compute Simpson's first rule on given mesh */
    result =  (*func)(mesh[0]) + (*func)(mesh[mesh.size()-1]) + 2 * (*func)(mesh[0] + h / 2);
    for(int i = 1; i < mesh.size()-1; i++)
        result = result + 2 * ((*func)(mesh[i]) + (*func)(mesh[i] + h / 2));

    /* Apply mesh refinement until we reach step size tolerance. This is triggered when 
       minStepSize is set to -1. */
    while((maxStep != -1) && (refinement < maxStep)) {

        /* Update step size */
        h = h / 2;
        
        /* Update refiniemt number */ 
        refinement++;

        /* Compute new Simpson's first rule evaluation for new midpoints (proper weights assigned later) */
        for(int i = 1; i <= pow(2, refinement+1) * (mesh.size() - 1); i=i+2) 
            result = result + 2 * (*func)(mesh[0] + i * h / 2);

        /* Exit refinement if we are at max iterations */
        if(refinement > MAX_REFINEMENT) {
            cout << "WARNING: Max refinement reached, exiting method." << endl;
            break;
        }

    }

    /* Add the additional weight to the midpoints */
    for(int i = 1; i <= pow(2, refinement+1) * (mesh.size() - 1); i=i+2) 
            result = result + 2 * (*func)(mesh[0] + i * h / 2);

    result = result * (h / 6);
    return result;

}

/* MIDPOINT RULE */
precision midpointRule(vector<precision> mesh, precision maxStep, precision (*func)(precision)) {

    precision result = 0;                                           /* Store result */
    precision h = mesh[1] - mesh[0];                                /* Step size */
    int refinement = 0;                                             /* Track refinement iterations */

    /* Compute midpoint rule on given mesh */
    for(int i = 0; i < mesh.size() - 1; i++) 
        result = result + (*func)(mesh[i] + h / 2);

    while((maxStep != -1) && (refinement < maxStep)) {

        /* Update step size */
        h = h / 3;

        /* Update refinement */
        refinement++;
        
        /* Integer to flag when to sum new midpoint evaluation, we iterate over all midpoints */
        for(int i = 0; i < pow(3,refinement) * (mesh.size() - 1); i++) {

            /* We are at a midpoint that has been accoiunted for already */
            if(((i + 2) % 3) != 0) 
                result = result + (*func)(mesh[0] + h / 2 + i * h); 

        }

        /* Exit refinement if we are at max iterations */
        if(refinement > MAX_REFINEMENT) {
            cout << "WARNING: Max refinement reached, exiting method." << endl;
            break;
        }

    }

    result = result * h;
    return result;

}

/* OPEN NEWTON COTES THAT USES 2 POINTS */
precision openNewtonCotes(vector<precision> mesh, precision maxStep, precision (*func)(precision)) {

    precision result = 0;                                           /* Store result */
    precision h = mesh[1] - mesh[0];                                /* Step size */
    int refinement = 0;                                             /* Track refinement iterations */

    /* Compute midpoint rule on given mesh */
    for(int i = 0   ; i < mesh.size() - 1; i++) 
        result = result + (*func)(mesh[i] + h / 3) + (*func)(mesh[i] + 2 * h / 3);

    while((maxStep != -1) && (refinement < maxStep)) {

        /* Update step size */
        h = h / 2;

        /* Update refinement */
        refinement++;

        /* Compute new evaluations for the refinement */
        for(int i = 0; i < pow(2, refinement-1) * (mesh.size() - 1); i++) 
            result = result + (*func)(mesh[0] + h / 3 + 2 * i * h) + (*func)(mesh[0] + 5 * h / 3 + 2 * i * h);

    }

    result = result * (h / 2);
    return result;

}

/* GAUSS LEGENDRE METHOD */
precision gaussLegendre(vector<precision> mesh, precision maxStep, precision (*func)(precision)) {

    precision result = 0;                                           /* Store result */
    precision h = mesh[1] - mesh[0];                                /* Step size */
    int refinement = 0;                                             /* Track refinement iterations */
    precision x1 = sqrt(1.0 / 3.0);                                 /* First x2  value */
    precision x2 = -1 * sqrt(1.0 / 3.0);                            /* Second x value */

    /* Compute Gauss Legendre for unrefined given mesh */
    for(int i = 0; i < mesh.size() - 1; i++) 
        result = result + (*func)((1.0 / 2.0) * (x1 * (mesh[i+1] - mesh[i]) + mesh[i] + mesh[i+1]))
                 + (*func)((1.0 / 2.0) * (-1 * x1 * (mesh[i+1] - mesh[i]) + mesh[i] + mesh[i+1]));

    while((maxStep != -1) && (refinement < maxStep)) {

        result = 0;

        /* Update step size */
        h = h / 2;

        /* Update refinement */
        refinement++;

        for(int i = 0; i < pow(2, refinement) * (mesh.size() - 1); i++)
            result = result + (*func)((1.0 / 2.0) * (2 * mesh[0] + (2 * i + 1) * h + h * x1))
                     + (*func)((1.0 / 2.0) * (2 * mesh[0] + (2 * i + 1) * h + h * x2));
    }

    result = result * (h / 2);
    return result;

}

/* =============== HELPER FUNCTIONS =============== */

/* UNIFORM MESH
   Creates a uniform mesh [leftBound, rightBound] with n mesh points */
vector<precision> uniformMesh(double leftBound, double rightBound, int n) {

    vector<precision> result;                           /* Stores result */
    
    /* Calculate step size and create mesh */
    precision h = (rightBound - leftBound)/ (n-1);
    for(int i = 0; i < n; i++)
        result.push_back(leftBound + i * h);
    
    return result;

}

/* MAIN FUNCTION */
int main(int argc, char *argv[]) {

    vector<precision> x = uniformMesh(0.1,2.5,83);
    int numRefinements = 1;

    cout << "==============TRAPEZOIDAL==============" << endl;
    cout << "=======================================" << endl << endl;

    cout << "Naive method : " << trapezoidalRule(x, -1, func5) << endl;

    for(int i = 1; i < numRefinements; i++) {
        cout << "Refined with " << i << " refinements - Result : " << trapezoidalRule(x,i,func1) << endl;
    }

    cout << endl;

    cout << "================SIMPSON================" << endl;
    cout << "=======================================" << endl << endl;

    cout << "Naive method : " << simpsonRule(x, -1, func5) << endl;

    for(int i = 1; i < numRefinements; i++) {
        cout << "Refined with " << i << " refinements - Result : " << simpsonRule(x,i,func1) << endl;
    }

    cout << endl;

    cout << "================MIDPOINT===============" << endl;
    cout << "=======================================" << endl << endl;

    cout << "Naive method : " << midpointRule(x, -1, func5) << endl;

    for(int i = 1; i < numRefinements; i++) {
        cout << "Refined with " << i << " refinements - Result : " << midpointRule(x,i,func1) << endl;
    }

    cout << endl;

    cout << "================OPEN-NC================" << endl;
    cout << "=======================================" << endl << endl;

    cout << "Naive method : " << openNewtonCotes(x, -1, func5) << endl;

    for(int i = 1; i < numRefinements; i++) {
        cout << "Refined with " << i << " refinements - Result : " << openNewtonCotes(x,i,func1) << endl;
    }

    cout << endl;

    cout << "============GAUSS-LEGENDRE=============" << endl;
    cout << "=======================================" << endl << endl;

    cout << "Naive method : " << gaussLegendre(x, -1, func5) << endl;

    for(int i = 1; i < numRefinements; i++) {
        cout << "Refined with " << i << " refinements - Result : " << gaussLegendre(x,i,func1) << endl;
    }

    return 0;

}