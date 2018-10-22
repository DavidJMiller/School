float dotprod(float x[], float y[], int n) {
    int i;
    float sum = 0;

    for(i = 0; i < n;i += 8) {
        sum += x[i] * y[i];
        sum += x[i+1] * y[i+1];
        sum += x[i+2] * y[i+2];
        sum += x[i+3] * y[i+3];
        sum += x[i+4] * y[i+4];
        sum += x[i+5] * y[i+5];
        sum += x[i+6] * y[i+6];
        sum += x[i+7] * y[i+7];
    }

    return sum;
}