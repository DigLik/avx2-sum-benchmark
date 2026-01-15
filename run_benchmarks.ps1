$ErrorActionPreference = "Stop"

function Check-Command ($cmd) {
    if (-not (Get-Command $cmd -ErrorAction SilentlyContinue)) {
        Write-Error "Команда '$cmd' не найдена. Убедитесь, что она установлена и добавлена в PATH."
        exit 1
    }
}

Write-Host "Проверка инструментов..." -ForegroundColor Cyan
Check-Command "dotnet"
Check-Command "gcc"
Check-Command "g++"

Write-Host "Все инструменты найдены. Начало тестов." -ForegroundColor Green
Write-Host "----------------------------------------"

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

Write-Host "`n[2/3] Компиляция и запуск C (GCC)..." -ForegroundColor Yellow
Push-Location src/c
try {
    gcc -O3 -mavx2 main.c -o benchmark
    
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
    Pop-Location
}

Write-Host "`n[3/3] Компиляция и запуск C++ (G++)..." -ForegroundColor Yellow
Push-Location src/cpp
try {
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
    Pop-Location
}

Write-Host "`n----------------------------------------"
Write-Host "Тесты завершены." -ForegroundColor Green
