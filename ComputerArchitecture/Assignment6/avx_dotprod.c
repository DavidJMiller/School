#include <immintrin.h>  

float dotprod(float x[], float y[], int n) {
    int i;
    float sums[8];
    __m256 sum_m256 = _mm256_setzero_ps();

    for(i = 0; i < n;i += 8) 
        sum_m256 = _mm256_add_ps(sum_m256, _mm256_mul_ps(_mm256_load_ps(x + i), _mm256_load_ps(y + i)));

    _mm256_store_ps(sums, sum_m256);
    return (sums[0] + sums[1] + sums[2] + sums[3] + sums[4] + sums[5] + sums[6] + sums[7]);
}