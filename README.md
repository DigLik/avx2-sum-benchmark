| Language | Compiler / Runtime | Best Time | Est. Bandwidth |
| :--- | :--- | :--- | :--- |
| **C#** | .NET 10.0 (JIT) | **23.40 ms** | **~34.1 GB/s** |
| **C++** | G++ `-O3` | 23.89 ms | ~33.4 GB/s |
| **C** | GCC `-O3` | 24.20 ms | ~33.0 GB/s |

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
