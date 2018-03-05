q   #ifdef DOUBLE
typedef double precision;
#else
typedef float precision;
#endif

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <array>
#include <cmath>
#include <limits>
#include <typeinfo>
#include <utility>

using namespace std;

/* ONLY ALTER CODE BETWEEN THE COMMENTING
CHANGE THE VALUES ACCORDING TO WHAAT WE HAVE
---------------------------------------*/

double leftBound = -1;
double rightBound = 1;
bool naturalBC = true;
bool firstHermite = false;
bool secondHermite = false;

precision leftFirstDerivative = 0.16528925;
precision rightFirstDerivative = -0.16528925;
precision leftSecondDerivative = -2;
precision rightSecondDerivative = -2;

double evaluateFunction(double xx) {
    //return (1 / (1 + 10*pow(xx, 2)));
    //return (abs(1*xx) + xx/2 - pow(xx,2));
    return sin(xx);
}

/*-------------------------------------
   DO NOT ALTER CODE BELOW THIS COMMENT*/


/* -------------------------------------- */
/* ----------------NEWTON---------------- */
/* -------------------------------------- */

/* DIVIDED DIFFERENCE */
void dividedDifference(precision* xMesh, precision* yMesh, precision* c, int degree) {

    for(int i = 0; i <= degree; i++)
        c[i] = yMesh[i];

    int level = 1;
    for(int i = 0; i < degree; i++) {
        for(int j = degree; j>i; j--) {
            c[j] = (c[j] - c[j-1]) / (xMesh[j] - xMesh[j-level]);
        }
        level++;
    }

    return;

}

/* NEWTON'S METHOD */
void newtonMethod(precision* xx, int numPoints, 
                  precision* xMesh, precision* yMesh, int degree, precision* results) {

    precision coefficients[degree+1];                       // stores the divided difference values
    dividedDifference(xMesh, yMesh, coefficients, degree);  // compute divided difference values

    // Iterate over x-points to evaluate at
    for(int i = 0; i < numPoints; i++) {

        // Set the result to the first divided difference 
        precision result = coefficients[0];

        //Iterate over degree of polynomial to build it
        for(int j = 1; j <= degree; j++) {

            // Initialize product value
            precision product = 1.0;

            // Iterate over current degree
            for(int k = 0; k < j; k++) {
               
                // Build the product iteratively
                product = product*(xx[i] - xMesh[k]);
            }
            // Multiply by respective divided difference value
            result += coefficients[j]*product;
        }

        // Store the result for the specified x-point
        results[i] = result;

    }

    return;

}

/* -------------------------------------- */
/* --------------BERNSTEIN--------------- */
/* -------------------------------------- */

/* HELPER FUNCTION FOR BERNSTEIN METHOD */
precision factorial(int n) {
    
    precision result = 1.0;
    if(n == 0)
        result = 1.0;
    else{
        for(int m = n; m > 0; m--)
            result = m*result;
    }
    return result;

}

/* HELPER FUNCTION FOR BERNSTEIN METHOD */
precision nChoosek(int n, int k) {
    return(factorial(n)/(factorial(k)*factorial(n-k)));
}

/* BERNSTEIN METHOD */
precision bernsteinMethod(precision xx, precision* yMesh, int degree) {

    precision result = 0.0;
    precision f = (xx - leftBound) / (rightBound - leftBound);
    for(int i = 0; i <= degree; i++)
        result += yMesh[i]*nChoosek(degree,i)*pow(f,i)*pow(1-f, degree-i);

    return result;
    
}

/* -------------------------------------- */
/* ---------------PIECEWISE-------------- */
/* -------------------------------------- */

/* HELPER FUNCTION FOR HEAPSORT */
void heapify(precision* xx, int n, int node, precision* yy) {
    int max = node;    
    int left = 2*node + 1;  
    int right = 2*node + 2;  

    if (left < n && xx[left] > xx[max])
        max = left;

    if (right < n && xx[right] > xx[max])
        max = right;

    if (max != node) {
        swap(xx[node], xx[max]);
        swap(yy[node], yy[max]);
        heapify(xx, n, max, yy);
    }
}

/* HEAPSORT */
void heapSort(precision* xx, int arraySize, precision* yy) {

    for (int i = arraySize / 2 - 1; i >= 0; i--)
        heapify(xx, arraySize, i, yy);

    for (int i = arraySize-1; i >= 0; i--) {
        swap(xx[0], xx[i]);
        swap(yy[0], yy[i]);
        heapify(xx, i, 0, yy);
    }
}

/* PIECEWISE METHOD */
void piecewiseMethod(precision* xx, int numPoints,
                     precision* xMesh, int meshSize,
                     precision* yMesh, int degree, precision* results) {

    // Return if we can not have all sub-intervals interpolated by same degree olynomial
    if((meshSize - 1) % degree != 0) {
        cout << "ERROR: All sub-intervals can not support polynomial of degree " << degree  << endl;
        return;
    }

    float subintervals = (meshSize - 1) / degree;       // num of piecewise intervals
    precision subxMesh[degree + 1];                     // submesh for x values
    precision subyMesh[degree + 1];                     // submesh for y values

    // Sort our points and their resepctive y values
    heapSort(xMesh, meshSize, yMesh);

    // Iterate over points we want to evaluate at
    for(int m = 0; m < numPoints; m++) {

        // Iterate over our mesh to find sub-interval we are in
        for(int j = 0; j < meshSize; j = j + degree) {

            // Check if we are in correct sub-interval
            if((xx[m] >= xMesh[j]) && (xx[m] <= xMesh[j+degree])) {
 
                // Get sub-interval points to pass to newtonMethod()
                for(int k = 0; k <= degree; k++) {
                    subxMesh[k] = xMesh[j + k];
                    subyMesh[k] = yMesh[j + k];
                }

                // Break since we already found correct sub-interval
                break;
            }
        }

    // Evaulate the x-point using newtonMethod() and store in result
    precision val[1];
    precision point[1] = {xx[m]};
    newtonMethod(point, 1, subxMesh, subyMesh, degree, val);

    results[m] = val[0];
    
    }

    return;
}

/* -------------------------------------- */
/* --------------CUBIC SPLINE------------ */
/* -------------------------------------- */

/* SOLVE TRIDIAGONAL MATRIX */
void solveTridig(precision* aa, precision bb, precision* cc, 
                 precision* dd, precision* results, int meshSize) {

    int n = meshSize - 1;

    cc[1] = cc[1]/bb;
    for(int i = 2; i < n-1; i++)
        cc[i] = cc[i] / (bb - aa[i]*cc[i-1]);

    dd[1] = dd[1]/bb;
    for(int i = 2; i < n; i++)
        dd[i] = (dd[i] - aa[i]*dd[i-1]) / (bb - aa[i]*cc[i-1]);

    results[n-1] = dd[n-1];
    for(int i = n-2; i > 0; i--)
        results[i] = dd[i] - cc[i]*results[i+1];

    return;

}

/* CUBIC SPLINE METHOD */
void cubicSpline(precision* xx, int numPoints, precision* xMesh, 
                 precision* yMesh, int meshSize, precision* results) {

    int n = meshSize - 1;           // Helpful int to keep around
    precision d[n+1] = {};            // Store d matrix
    precision lambda[n] = {};       // Store lambda values
    precision mu[n+1] = {};           // Store mu values
    precision M[n+1] = {};          // Store s'' matrix
    precision gamma[n] = {};        // Store gamma values
    precision gammaTilde[n] = {};   // Store gammaTilde values

    // Sort the mesh so that x_0 < x_1 < ...
    heapSort(xMesh, meshSize, yMesh);

    // Compute mu, lambda, and d values
    for(int i = 1; i < n; i++) {
        mu[i] = (xMesh[i] - xMesh[i-1]) / (xMesh[i+1] - xMesh[i-1]);
        lambda[i] = (xMesh[i+1] - xMesh[i]) / (xMesh[i+1] - xMesh[i-1]);
        d[i] = 6 * (((yMesh[i+1] - yMesh[i]) / (xMesh[i+1] - xMesh[i])) - 
               ((yMesh[i] - yMesh[i-1]) / (xMesh[i] - xMesh[i-1]))) / (xMesh[i+1] - xMesh[i-1]);
    }

    // Check if we have first Hermite boundary conditions
    if(firstHermite) {
        cout << "We have first hermite boundary conditions!" << endl;
        precision deltaOne = xMesh[1]-xMesh[0];
        precision deltaN = xMesh[n]-xMesh[n-1];
        lambda[0] = 1;
        mu[n] = 1;
        d[0] = 6 * ( (yMesh[1]-yMesh[0])/deltaOne - leftFirstDerivative) / deltaOne;
        d[n] = 6 * (leftSecondDerivative - (yMesh[n] - yMesh[n-1]) / deltaN) / deltaN;
    }

    // Check if we have second Hermite boundary conditions
    if(secondHermite) {
        cout << "We have second hermite boundary conditions!" << endl;
        M[0] = leftSecondDerivative;
        M[n] = rightSecondDerivative;
        d[1] = d[1] - mu[1] * leftSecondDerivative;
        d[n-1] = d[n-1] - lambda[n-1] * rightSecondDerivative;
    }

    // Solve tri-diagonal matrix
    solveTridig(mu, 2, lambda, d, M, meshSize);

    // Compute gamma values
    for(int i = 1; i <= n; i++) {
        gamma[i-1] = (yMesh[i] - yMesh[i-1]) / (xMesh[i] - xMesh[i-1]) - 
                     ((xMesh[i] - xMesh[i-1]) / 6) * (M[i] - M[i-1]);
    }

    // Compute gammaTilde values
    for(int i = 1; i <= n; i++) {
        gammaTilde[i-1] = yMesh[i-1] - M[i-1] * (pow(xMesh[i] - xMesh[i-1], 2) / 6);
    }

    // Iterate over points we are evaluating at
    for(int i = 0; i < numPoints; i++) {

        // Iterate over mesh to determine subinterval
        for(int j = 0; j < meshSize-1; j++) {

            // We found correct subinteval, calculate result and break after
            if(xx[i] >= xMesh[j] && xx[i] <= xMesh[j+1]) {
                results[i] = M[j] * pow(xMesh[j+1] - xx[i], 3) / (6 * (xMesh[j+1] - xMesh[j])) + 
                             M[j+1] * pow(xx[i] - xMesh[j], 3) / (6 * (xMesh[j+1] - xMesh[j])) + 
                             gamma[j] * (xx[i] - xMesh[j]) + gammaTilde[j];
                break;
            }
        }
    }

    return;
}

/* Helper function to create uniform mesh */
void uniformMesh(precision* xMesh, int meshSize, double leftBound, double rightBound) {
    
    for(int i = 0; i < meshSize; i++) 
        xMesh[i] = leftBound +  i * (rightBound - leftBound) / (meshSize - 1);

    return;

}

/* MAIN FUNCTION */
int main(int argc, char *argv[]) {

    // ---------- BERNESTEIN ----------
    /*    
    for(int i = 0; i <= 6; i++) {
        xMesh[i] = leftBound + i * (rightBound - leftBound) / (6);
        cout << xMesh[i] << endl;
        yMesh[i] = evaluateFunction(xMesh[i]);
    }
    for(int i =0; i <101; i++) {
        testPoints[i] = leftBound + i * (rightBound - leftBound) / (100);
    }
    for(int i = 0; i < 101; i++) {
        myfile << testPoints[i] << " " << bernsteinMethod(testPoints[i], yMesh, 6) << endl;
    }
    */

    // ---------- PIECEWISE & NEWTON ----------
    /* 
    precision testX[10] = {-1, -0.8, -0.6, -0.4, -0.2, 0.2, 0.4, 0.6, 0.8, 1};
    precision testY[10]; 
    for(int i = 0; i < 10; i++) 
        testY[i] = evaluateFunction(testX[i]);   
    
    precision xPnts[11];
    for(int i = 0; i < 11; i++) 
        xPnts[i] = leftBound + i * (rightBound - leftBound) / 10;

    precision results[11];
    piecewiseMethod(xPnts, 11, testX, 10, testY, 3, results); 

    for(int i = 0; i < 11; i++) {
        myfile << xPnts[i] << " " << results[i] << endl;
    }
    */

    // ---------- CUBIC SPLINE ----------
    /* 
    precision results[501];
    cubicSpline(testPoints, 501, xMesh, yMesh, 5, results);

    for(int i = 0; i < 501; i++)
        myfile << testPoints[i] << " " << results[i] << endl;

    ofstream myfile, errorPtr, meshPtr;
    myfile.open("allData.txt");
    errorPtr.open("error.txt"); 
    meshPtr.open("mesh.txt");

    
    int mesh = 22;
    int numPoints = 501;
    int interpolatingDegree = 3;

    precision resultsNM[numPoints];
    precision resultsPW[numPoints];
    precision resultsBM[numPoints];
    precision resultsCS[numPoints];
    
    precision testPoints[numPoints];
    precision values[numPoints];
    precision xMesh[mesh];
    precision yMesh[mesh];

    uniformMesh(xMesh, mesh, leftBound, rightBound);

    for(int i = 0; i < mesh; i++)
        yMesh[i] = evaluateFunction(xMesh[i]);

    for(int i =0; i < numPoints; i++) {
        testPoints[i] = leftBound + i * (rightBound - leftBound) / (numPoints - 1);
        values[i] = evaluateFunction(testPoints[i]);
    }


    newtonMethod(testPoints, numPoints, xMesh, yMesh, mesh-1, resultsNM);
    for(int i = 0; i < numPoints; i++) {
        resultsBM[i] = bernsteinMethod(testPoints[i], yMesh, mesh-1);
    }
    piecewiseMethod(testPoints, numPoints, xMesh, mesh, yMesh, interpolatingDegree, resultsPW);
    cubicSpline(testPoints, numPoints, xMesh, yMesh, mesh, resultsCS);

    for(int i = 0; i < numPoints; i++) {
        myfile << testPoints[i] << " " << values[i] << " ";
        myfile << resultsNM[i] << " " << resultsBM[i] << " ";
        myfile << resultsPW[i]  << " " << resultsCS[i] << endl;
    }

    for(int i = 0; i < numPoints; i++) {
        errorPtr << testPoints[i] << " ";
        errorPtr << abs(values[i] - resultsNM[i]) << " ";
        errorPtr << abs(values[i] - resultsBM[i]) << " ";
        errorPtr << abs(values[i] - resultsPW[i]) << " ";
        errorPtr << abs(values[i] - resultsCS[i]) << " " << endl;
    }

    precision meshY1[5];
    precision meshY2[15];
    precision meshY3[25];
    precision meshY4[35];
    precision meshY5[45];
    precision meshY6[55];
    for(int i = 0; i < 5; i++) {
        meshY1[i] = evaluateFunction(leftBound + i * (rightBound - leftBound) / (4));
    }
    for(int i = 0; i < 15; i++) {
        meshY2[i] = evaluateFunction(leftBound + i * (rightBound - leftBound) / (14));
    }
    for(int i = 0; i < 25; i++) {
        meshY3[i] = evaluateFunction(leftBound + i * (rightBound - leftBound) / (24));
    }
    for(int i = 0; i < 35; i++) {
        meshY4[i] = evaluateFunction(leftBound + i * (rightBound - leftBound) / (34));
    }
    for(int i = 0; i < 45; i++) {
        meshY5[i] = evaluateFunction(leftBound + i * (rightBound - leftBound) / (44));
    }
    for(int i = 0; i < 55; i++) {
        meshY6[i] = evaluateFunction(leftBound + i * (rightBound - leftBound) / (54));
    }
    precision resultsBMmesh1[numPoints];
    precision resultsBMmesh2[numPoints];
    precision resultsBMmesh3[numPoints];
    precision resultsBMmesh4[numPoints];
    precision resultsBMmesh5[numPoints];
    precision resultsBMmesh6[numPoints];
    for(int j = 0; j < numPoints; j++) {
        resultsBMmesh1[j] = bernsteinMethod(testPoints[j], meshY1, 4);
        resultsBMmesh2[j] = bernsteinMethod(testPoints[j], meshY2, 14);
        resultsBMmesh3[j] = bernsteinMethod(testPoints[j], meshY3, 24);
        resultsBMmesh4[j] = bernsteinMethod(testPoints[j], meshY4, 34);
        resultsBMmesh5[j] = bernsteinMethod(testPoints[j], meshY5, 44);
        resultsBMmesh6[j] = bernsteinMethod(testPoints[j], meshY6, 54);
    }

    for(int i = 0; i < numPoints; i++) {
        meshPtr << testPoints[i] << " " << values[i] << " ";
        meshPtr << resultsBMmesh1[i] << " ";
        meshPtr << resultsBMmesh2[i] << " ";
        meshPtr << resultsBMmesh3[i] << " ";
        meshPtr << resultsBMmesh4[i] << " ";
        meshPtr << resultsBMmesh5[i] << " ";
        meshPtr << resultsBMmesh6[i] << endl;
    }

    myfile.close();
    errorPtr.close();
    meshPtr.close();
    */

    /* ---------- ZHIFENG CODE ---------- */

    /* Open files that contain xMesh, yMesh, chebyshev xMesh, chebyshev yMesh */
    ifstream xData, yData, xcData, ycData;
    ofstream dataFile;
    xData.open("xData.txt");
    yData.open("yData.txt");
    xcData.open("xcData.txt");
    ycData.open("ycData.txt");
    dataFile.open("data.txt");

    /* Where the data is stored */
    int numInterpPoints = 21;
    precision x, y, xc, yc;
    precision xMesh[numInterpPoints];
    precision yMesh[numInterpPoints];
    precision xcMesh[numInterpPoints];
    precision ycMesh[numInterpPoints];

    /* Read in data */
    for(int i = 0; i < numInterpPoints; i++) {
        xData >> x;
        xMesh[i] = x;
        yData >> y;
        yMesh[i] = y;
        xcData >> xc;
        xcMesh[i] = xc;
        ycData >> yc;
        ycMesh[i] = yc;
    }
    
    /* Uniform grid of points to evaluate at */
    int numXPoints = 101;
    precision xPoints[numXPoints];
    for(int i =0; i < numXPoints; i++)
        xPoints[i] = leftBound + i * (rightBound - leftBound) / (numXPoints - 1);
    
    int celineDegree = 5;
    precision celineX[celineDegree+1] = {0.0, 0.0, 1.5, 1.5, 2.0, 2.0};
    precision celineY[celineDegree+1] = {1.0, 1.0, 0.0707, 0.0707, -0.4162, -0.4162};
    precision celineR[celineDegree+1];
    dividedDifference(celineX, celineY, celineR, celineDegree);
    cout<< endl << "============== CELINE STUFF ================" << endl << endl;
    for(int i = 0; i <= celineDegree; i++){
        cout << "Coefficient : " << celineR[i] << endl;
    }
    cout << endl;

    /* Store results for (xMesh,yMesh) and (chebyshev xMesh, chebyshev yMesh) */
    precision results[numXPoints];
    precision resultsCheb[numXPoints];

    /* Pass in data and store results */
    newtonMethod(xPoints, numXPoints, xMesh, yMesh, 20, results);
    newtonMethod(xPoints, numXPoints, xcMesh, ycMesh, 20, resultsCheb);

    /* Write results to file to plot */
    for(int i = 0; i < numXPoints; i++) {
        dataFile << xPoints[i] << " " << results[i] << " " << resultsCheb[i] << endl; 
    }

    /* Close out the files */
    xData.close();
    yData.close();
    xcData.close();
    ycData.close();
    dataFile.close();

    return 0;

}