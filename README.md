| Language | Compiler / Runtime | Best Time | Est. Bandwidth |
| :--- | :--- | :--- | :--- |
| **C#** | .NET 9.0 (AOT/JIT) | **23.40 ms** | **~34.1 GB/s** |
| **C++** | G++ (MinGW) `-O3` | 23.89 ms | ~33.4 GB/s |
| **C** | GCC (MinGW) `-O3` | 24.20 ms | ~33.0 GB/s |

```
cd src/csharp
dotnet run -c Release
```

```
cd src/c
gcc -O3 -mavx2 main.c -o benchmark
./benchmark
```

```
cd src/cpp
g++ -O3 -mavx2 main.cpp -o benchmark
./benchmark
```