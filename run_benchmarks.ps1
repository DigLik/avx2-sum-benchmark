# run_benchmarks.ps1

# Остановка скрипта при ошибках (опционально, но полезно для отладки)
$ErrorActionPreference = "Stop"

function Check-Command ($cmd) {
    if (-not (Get-Command $cmd -ErrorAction SilentlyContinue)) {
        Write-Error "Команда '$cmd' не найдена. Убедитесь, что она установлена и добавлена в PATH."
        exit 1
    }
}

# 1. Проверка наличия инструментов
Write-Host "Проверка инструментов..." -ForegroundColor Cyan
Check-Command "dotnet"
Check-Command "gcc"
Check-Command "g++"

Write-Host "Все инструменты найдены. Начало тестов." -ForegroundColor Green
Write-Host "----------------------------------------"

# 2. Запуск C#
Write-Host "`n[1/3] Запуск C# (.NET)..." -ForegroundColor Yellow
Push-Location src/csharp
try {
    dotnet run -c Release
}
catch {
    Write-Error "Ошибка при запуске C#"
}
finally {
    Pop-Location
}

# 3. Запуск C
Write-Host "`n[2/3] Компиляция и запуск C (GCC)..." -ForegroundColor Yellow
Push-Location src/c
try {
    # Компиляция
    gcc -O3 -mavx2 main.c -o benchmark
    
    # Проверка, создался ли файл (для Windows это будет benchmark.exe)
    if (Test-Path "benchmark.exe") {
        ./benchmark.exe
    } elseif (Test-Path "benchmark") {
        ./benchmark
    } else {
        Write-Error "Не удалось найти исполняемый файл C после компиляции."
    }
}
catch {
    Write-Error "Ошибка при сборке или запуске C"
}
finally {
    # Очистка (опционально)
    # Remove-Item benchmark.exe -ErrorAction SilentlyContinue
    Pop-Location
}

# 4. Запуск C++
Write-Host "`n[3/3] Компиляция и запуск C++ (G++)..." -ForegroundColor Yellow
Push-Location src/cpp
try {
    # Компиляция
    g++ -O3 -mavx2 main.cpp -o benchmark
    
    if (Test-Path "benchmark.exe") {
        ./benchmark.exe
    } elseif (Test-Path "benchmark") {
        ./benchmark
    } else {
        Write-Error "Не удалось найти исполняемый файл C++ после компиляции."
    }
}
catch {
    Write-Error "Ошибка при сборке или запуске C++"
}
finally {
    # Очистка (опционально)
    # Remove-Item benchmark.exe -ErrorAction SilentlyContinue
    Pop-Location
}

Write-Host "`n----------------------------------------"
Write-Host "Тесты завершены." -ForegroundColor Green