#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef _WIN32
    #include <windows.h>
    #include <malloc.h>
    
    void* aligned_alloc_wrapper(size_t alignment, size_t size) {
        return _aligned_malloc(size, alignment);
    }
    
    void aligned_free_wrapper(void* ptr) {
        _aligned_free(ptr);
    }

    typedef LARGE_INTEGER Timer;
    void timer_start(Timer* t) { QueryPerformanceCounter(t); }
    double timer_elapsed_ms(Timer start) {
        LARGE_INTEGER end, freq;
        QueryPerformanceCounter(&end);
        QueryPerformanceFrequency(&freq);
        return (double)(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    }

#else
    #include <time.h>
    
    void* aligned_alloc_wrapper(size_t alignment, size_t size) {
        return aligned_alloc(alignment, size);
    }
    
    void aligned_free_wrapper(void* ptr) {
        free(ptr);
    }

    typedef struct timespec Timer;
    void timer_start(Timer* t) { clock_gettime(CLOCK_MONOTONIC, t); }
    double timer_elapsed_ms(Timer start) {
        struct timespec end;
        clock_gettime(CLOCK_MONOTONIC, &end);
        return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    }
#endif

typedef struct {
    int64_t Sum;
    double Elapsed;
} Result;

int32_t hsum_avx2(__m256i v) {
    __m128i lo = _mm256_castsi256_si128(v);
    __m128i hi = _mm256_extracti128_si256(v, 1);
    __m128i sum128 = _mm_add_epi32(lo, hi);
    sum128 = _mm_hadd_epi32(sum128, sum128);
    sum128 = _mm_hadd_epi32(sum128, sum128);
    return _mm_cvtsi128_si32(sum128);
}

int main() {
    const uint32_t Size = 200000000;
    const uint32_t Iterations = 50;

    Result* results = (Result*)malloc(Iterations * sizeof(Result));
    int* array = (int*)aligned_alloc_wrapper(32, Size * sizeof(int));

    for (uint32_t k = 0; k < Size; k++) {
        array[k] = 2;
    }

    const int VectorCount = 8; // 256 бит / 32 бита int = 8 элементов

    for (uint32_t i = 0; i < Iterations; i++) {
        __m256i sum0 = _mm256_setzero_si256();
        __m256i sum1 = _mm256_setzero_si256();
        __m256i sum2 = _mm256_setzero_si256();
        __m256i sum3 = _mm256_setzero_si256();

        int64_t finalSum = 0;
        uint32_t j = 0;

        const uint32_t Step = VectorCount * 4;

        Timer startTimestamp;
        timer_start(&startTimestamp);

        for (; j <= Size - Step; j += Step) {
            const int* ptr = array + j;

            __m256i v0 = _mm256_load_si256((const __m256i*)ptr);
            __m256i v1 = _mm256_load_si256((const __m256i*)(ptr + VectorCount));
            __m256i v2 = _mm256_load_si256((const __m256i*)(ptr + VectorCount * 2));
            __m256i v3 = _mm256_load_si256((const __m256i*)(ptr + VectorCount * 3));

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
            __m256i v = _mm256_load_si256((const __m256i*)(array + j));
            combinedSum = _mm256_add_epi32(combinedSum, v);
        }

        finalSum += hsum_avx2(combinedSum);

        for (; j < Size; j++) {
            finalSum += array[j];
        }

        double durationMs = timer_elapsed_ms(startTimestamp);
        results[i].Sum = finalSum;
        results[i].Elapsed = durationMs;
    }

    for (uint32_t i = 0; i < Iterations; i++) {
        printf("Iteration %u: Sum = %lld, Elapsed = %.4f ms\n", i + 1, results[i].Sum, results[i].Elapsed);
    }

    aligned_free_wrapper(array);
    free(results);

    return 0;
}