#include <iostream>
#include <vector>
#include <chrono>
#include <immintrin.h>
#include <cstdint>
#include <iomanip>
#include <algorithm>

struct Result {
    int64_t Sum;
    double Elapsed;
};

int32_t hsum_avx2(__m256i v) {
    __m128i vlow = _mm256_castsi256_si128(v);
    __m128i vhigh = _mm256_extracti128_si256(v, 1);
    __m128i v128 = _mm_add_epi32(vlow, vhigh);

    v128 = _mm_hadd_epi32(v128, v128);
    // (a+b, c+d) -> (a+b+c+d)
    v128 = _mm_hadd_epi32(v128, v128);

    return _mm_cvtsi128_si32(v128);
}

int main() {
    const uint32_t Size = 200'000'000;
    const uint32_t Iterations = 50;
    const int VectorCount = 8;

    std::vector<Result> results(Iterations);
    int* array = static_cast<int*>(_mm_malloc(Size * sizeof(int), 32));

    std::fill_n(array, Size, 2);

    const uint32_t Step = VectorCount * 4;

    for (uint32_t i = 0; i < Iterations; i++) {
        __m256i sum0 = _mm256_setzero_si256();
        __m256i sum1 = _mm256_setzero_si256();
        __m256i sum2 = _mm256_setzero_si256();
        __m256i sum3 = _mm256_setzero_si256();

        int64_t finalSum = 0;
        uint32_t j = 0;

        auto startTimestamp = std::chrono::high_resolution_clock::now();

        const int* ptr = array;
        for (; j <= Size - Step; j += Step) {
            __m256i v0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(ptr + j));
            __m256i v1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(ptr + j + VectorCount));
            __m256i v2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(ptr + j + VectorCount * 2));
            __m256i v3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(ptr + j + VectorCount * 3));

            sum0 = _mm256_add_epi32(sum0, v0);
            sum1 = _mm256_add_epi32(sum1, v1);
            sum2 = _mm256_add_epi32(sum2, v2);
            sum3 = _mm256_add_epi32(sum3, v3);
        }

        __m256i combinedSum = _mm256_add_epi32(
            _mm256_add_epi32(sum0, sum1),
            _mm256_add_epi32(sum2, sum3)
        );

        for (; j <= Size - VectorCount; j += VectorCount) {
            __m256i v = _mm256_load_si256(reinterpret_cast<const __m256i*>(array + j));
            combinedSum = _mm256_add_epi32(combinedSum, v);
        }

        finalSum += hsum_avx2(combinedSum);

        for (; j < Size; j++) {
            finalSum += array[j];
        }

        auto endTimestamp = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = endTimestamp - startTimestamp;

        results[i] = { finalSum, duration.count() };
    }

    std::cout << std::fixed << std::setprecision(4);
    for (uint32_t i = 0; i < Iterations; i++) {
        std::cout << "Iteration " << (i + 1) 
                  << ": Sum = " << results[i].Sum 
                  << ", Elapsed = " << results[i].Elapsed << " ms" << std::endl;
    }

    _mm_free(array);

    return 0;
}