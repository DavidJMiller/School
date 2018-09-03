float dotprod(float x[], float y[], int n) {
    int i;
    float sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0, sum6 = 0, sum7 = 0, sum8 = 0;

    for(i = 0; i < n;i += 8) {
        sum1 = sum1 + x[i] * y[i];
        sum2 = sum2 + x[i+1] * y[i+1];
        sum3 = sum3 + x[i+2] * y[i+2];
        sum4 = sum4 + x[i+3] * y[i+3];
        sum5 = sum5 + x[i+4] * y[i+4];
        sum6 = sum6 + x[i+5] * y[i+5];
        sum7 = sum7 + x[i+6] * y[i+6];
        sum8 = sum8 + x[i+7] * y[i+7];
    }

    return (sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8);
}