param(
    [ValidateSet("Build", "Clean", "Help")]
    [string]$Target = "Build"
)

$AppName = "sdltimer"
$SrcFiles = @("main.c", "timer.c")
$IncludeDirs = @("include")
$ObjDir = "build"
$OutDir = "."

function Find-Msvc {
    $cl = Get-Command "cl.exe" -ErrorAction SilentlyContinue
    if ($cl) { return $cl.Source }

    $vsPaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build"
    )
    foreach ($p in $vsPaths) {
        $bat = Join-Path $p "vcvars64.bat"
        if (Test-Path $bat) { return $bat }
    }
    return $null
}

function Find-MinGW {
    $gcc = Get-Command "gcc" -Source -ErrorAction SilentlyContinue
    if ($gcc) { return $gcc.Source }

    $mingwPaths = @(
        "${env:ProgramFiles}\mingw64\bin\gcc.exe",
        "${env:ProgramFiles}\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin\gcc.exe",
        "C:\msys64\mingw64\bin\gcc.exe",
        "C:\tools\mingw64\bin\gcc.exe"
    )
    foreach ($p in $mingwPaths) {
        if (Test-Path $p) { return $p }
    }
    return $null
}

function Find-SDL3 {
    $sdlPaths = @(
        "${env:SDL3_DIR}",
        "${env:SDL_DIR}",
        "C:\SDL3",
        "C:\SDL\SDL3",
        "${env:ProgramFiles}\SDL3",
        "${env:ProgramFiles(x86)}\SDL3"
    )

    $result = $null

    if ($env:SDL3_DIR) {
        $d = $env:SDL3_DIR
        if (Test-Path (Join-Path $d "include\SDL3\SDL.h")) {
            $result = $d
        }
    }

    if (-not $result) {
        foreach ($p in $sdlPaths) {
            if ($p -and (Test-Path (Join-Path $p "include\SDL3\SDL.h"))) {
                $result = $p
                break
            }
        }
    }

    return $result
}

function Invoke-MsvcBuild {
    $sdlDir = Find-SDL3
    if (-not $sdlDir) {
        Write-Error "SDL3 not found. Set `$env:SDL3_DIR to the SDL3 root, or install SDL3 in a standard location."
        return $false
    }

    $sdlInclude = Join-Path $sdlDir "include"
    $sdlLib = Join-Path $sdlDir "lib"

    $cl = Find-Msvc
    if (-not $cl) {
        Write-Error "MSVC not found. Run from a Developer Command Prompt or install Visual Studio."
        return $false
    }

    $clExe = if ($cl -like "*vcvars*.bat") {
        "cl.exe"
    } else {
        $cl
    }

    $cflags = "/nologo /W4 /Iinclude /I$sdlInclude /DGIT_HASH=`"unknown`""
    $ldflags = "/link /LIBPATH:$sdlLib SDL3.lib"

    if (-not (Test-Path $ObjDir)) { New-Item -ItemType Directory -Path $ObjDir | Out-Null }

    $objFiles = @()
    foreach ($src in $SrcFiles) {
        $obj = Join-Path $ObjDir "$([System.IO.Path]::GetFileNameWithoutExtension($src)).obj"
        $objFiles += $obj
        Write-Host "  CC $src"
        & $clExe /c $cflags /Fo$obj $src
        if ($LASTEXITCODE -ne 0) { return $false }
    }

    Write-Host "  LINK $AppName.exe"
    & $clExe $cflags $($objFiles -join " ") $ldflags /Fe"$AppName.exe"
    if ($LASTEXITCODE -ne 0) { return $false }

    return $true
}

function Invoke-MinGWBuild {
    $sdlDir = Find-SDL3
    if (-not $sdlDir) {
        Write-Error "SDL3 not found. Set `$env:SDL3_DIR to the SDL3 root, or install SDL3 in a standard location."
        return $false
    }

    $sdlInclude = Join-Path $sdlDir "include"
    $sdlLib = Join-Path $sdlDir "lib"

    $gcc = Find-MinGW
    if (-not $gcc) {
        Write-Error "MinGW-w64 (gcc) not found. Install it or add it to PATH."
        return $false
    }

    $cflags = "-Wall -Wextra -Iinclude -I$sdlInclude -DGIT_HASH=`"unknown`""
    $ldflags = "-L$sdlLib -lSDL3"

    if (-not (Test-Path $ObjDir)) { New-Item -ItemType Directory -Path $ObjDir | Out-Null }

    $objFiles = @()
    foreach ($src in $SrcFiles) {
        $obj = Join-Path $ObjDir "$([System.IO.Path]::GetFileNameWithoutExtension($src)).o"
        $objFiles += $obj
        Write-Host "  CC $src"
        & $gcc -c $cflags -o $obj $src
        if ($LASTEXITCODE -ne 0) { return $false }
    }

    Write-Host "  LINK $AppName.exe"
    & $gcc $cflags -o "$AppName.exe" $($objFiles -join " ") $ldflags
    if ($LASTEXITCODE -ne 0) { return $false }

    return $true
}

function Build {
    $compiler = $null

    if (Find-Msvc) { $compiler = "msvc" }
    elseif (Find-MinGW) { $compiler = "mingw" }

    if (-not $compiler) {
        Write-Error "No supported C compiler found. Install Visual Studio (MSVC) or MinGW-w64."
        return
    }

    Write-Host "=== Building $AppName ($compiler) ==="

    if ($compiler -eq "msvc") {
        $ok = Invoke-MsvcBuild
    } else {
        $ok = Invoke-MinGWBuild
    }

    if ($ok) {
        Write-Host "=== Build succeeded: $AppName.exe ==="
    } else {
        Write-Error "=== Build failed ==="
    }
}

function Clean {
    Write-Host "=== Cleaning ==="
    if (Test-Path $ObjDir) {
        Remove-Item -Recurse -Force $ObjDir
        Write-Host "  Removed $ObjDir/"
    }
    $bin = "$AppName.exe"
    if (Test-Path $bin) {
        Remove-Item -Force $bin
        Write-Host "  Removed $bin"
    }
    Write-Host "=== Done ==="
}

function Help {
    Write-Host "Usage: .\build.ps1 [[-Target] Build|Clean|Help]"
    Write-Host
    Write-Host "Targets:"
    Write-Host "  Build   - Compile and link $AppName.exe (default)"
    Write-Host "  Clean   - Remove build artifacts"
    Write-Host "  Help    - Show this message"
    Write-Host
    Write-Host "Prerequisites:"
    Write-Host "  - SDL3 (set `$env:SDL3_DIR or install to C:\SDL3)"
    Write-Host "  - MSVC (Visual Studio) or MinGW-w64 (gcc) in PATH"
}

switch ($Target) {
    "Build" { Build }
    "Clean" { Clean }
    "Help"  { Help }
}
