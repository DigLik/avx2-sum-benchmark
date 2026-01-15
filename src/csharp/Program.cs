using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Intrinsics.X86;

const uint Size = 200_000_000;
const uint Iterations = 50;

unsafe
{
    Result* results = (Result*)NativeMemory.Alloc(Iterations, Result.Size);
    int* array = (int*)NativeMemory.AlignedAlloc(Size * sizeof(int), 32);

    new Span<int>(array, (int)Size).Fill(2);

    const int VectorCount = 8;

    for (uint i = 0; i < Iterations; i++)
    {
        Vector256<int> sum0 = Vector256<int>.Zero;
        Vector256<int> sum1 = Vector256<int>.Zero;
        Vector256<int> sum2 = Vector256<int>.Zero;
        Vector256<int> sum3 = Vector256<int>.Zero;

        long finalSum = 0;
        uint j = 0;

        const uint Step = VectorCount * 4;

        long startTimestamp = Stopwatch.GetTimestamp();

        for (; j <= Size - Step; j += Step)
        {
            int* ptr = array + j;

            Vector256<int> v0 = Avx.LoadAlignedVector256(ptr);
            Vector256<int> v1 = Avx.LoadAlignedVector256(ptr + VectorCount);
            Vector256<int> v2 = Avx.LoadAlignedVector256(ptr + VectorCount * 2);
            Vector256<int> v3 = Avx.LoadAlignedVector256(ptr + VectorCount * 3);

            sum0 = Avx2.Add(sum0, v0);
            sum1 = Avx2.Add(sum1, v1);
            sum2 = Avx2.Add(sum2, v2);
            sum3 = Avx2.Add(sum3, v3);
        }

        Vector256<int> combinedSum = Avx2.Add(Avx2.Add(sum0, sum1), Avx2.Add(sum2, sum3));

        for (; j <= Size - VectorCount; j += VectorCount)
        {
            Vector256<int> v = Avx.LoadAlignedVector256(array + j);
            combinedSum = Avx2.Add(combinedSum, v);
        }

        finalSum += Vector256.Sum(combinedSum);

        for (; j < Size; j++)
            finalSum += array[j];

        TimeSpan duration = Stopwatch.GetElapsedTime(startTimestamp);
        results[i] = new Result(finalSum, duration.TotalMilliseconds);
    }

    for (uint i = 0; i < Iterations; i++)
        Console.WriteLine($"Iteration {i + 1}: Sum = {results[i].Sum}, Elapsed = {results[i].Elapsed:F4} ms");

    NativeMemory.AlignedFree(array);
    NativeMemory.Free(results);
}

readonly record struct Result(long Sum, double Elapsed)
{
    public const uint Size = sizeof(long) + sizeof(double);
}
