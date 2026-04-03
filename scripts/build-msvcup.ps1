# Build script using msvcup toolchain (no Visual Studio required)
# Usage: .\scripts\build-msvcup.ps1 [-UnrealPath "C:\unreal\UE_5.7"] [-Target "CoopGameFleep"] [-Configuration "Development"]
#
# Prerequisites: msvcup.exe must be available (downloaded automatically if not present)
# This script:
#   1. Downloads msvcup if not found in tools/
#   2. Installs MSVC 14.44 + Windows SDK 10.0.22621 via msvcup
#   3. Sets up UE AutoSDK directory junctions so UBT discovers the toolchain
#   4. Sets UE_SDKS_ROOT and activates vcvars environment
#   5. Invokes UBT Build.bat

param(
    [string]$UnrealPath = "",
    [string]$ProjectPath = "",
    [string]$ProjectName = "CoopGameFleep.uproject",
    [string]$Target = "CoopGameFleep",
    [string]$Platform = "Win64",
    [string]$Configuration = "Development",
    [string]$MsvcVersion = "msvc-14.44.17.14",
    [string]$SdkVersion = "sdk-10.0.22621.7"
)

$ErrorActionPreference = "Stop"

# --- Resolve paths ---
$ScriptDir = $PSScriptRoot
$ProjectRoot = Split-Path $ScriptDir -Parent
if ([string]::IsNullOrEmpty($ProjectPath)) { $ProjectPath = $ProjectRoot }

function Resolve-UnrealPath {
    param([string]$PathFromArgs)
    if (-not [string]::IsNullOrEmpty($PathFromArgs)) { return $PathFromArgs }
    $hostname = [System.Net.Dns]::GetHostName()
    switch -Regex ($hostname.ToLowerInvariant()) {
        "^filfreire01$"     { return "C:\unreal\UE_5.7" }
        "^filfreire02$"     { return "D:\unreal\UE_5.7" }
        "^desktop-doap6m9$" { return "E:\unreal\UE_5.7" }
        "^unreal-"          { return "C:\unreal\UE_5.7" }
        default             { return "D:\unreal\UE_5.7" }
    }
}

$ResolvedUnrealPath = Resolve-UnrealPath -PathFromArgs $UnrealPath
$ToolsDir   = Join-Path $ProjectRoot "tools"
$MsvcupExe  = Join-Path $ToolsDir "msvcup.exe"
$MsvcDir    = Join-Path $ToolsDir "msvc"
$AutoSdkDir = Join-Path $ToolsDir "autosdk"

# --- Step 1: Download msvcup if not present ---
if (-not (Test-Path $MsvcupExe)) {
    Write-Host "Downloading msvcup..." -ForegroundColor Cyan
    New-Item -ItemType Directory -Path $ToolsDir -Force | Out-Null
    $zipPath = Join-Path $ToolsDir "msvcup.zip"
    $arch = if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") { "aarch64" } else { "x86_64" }
    $url = "https://github.com/marler8997/msvcup/releases/download/v2026_03_02/msvcup-${arch}-windows.zip"
    Invoke-WebRequest -Uri $url -OutFile $zipPath
    Expand-Archive $zipPath -DestinationPath $ToolsDir -Force
    Remove-Item $zipPath
    if (-not (Test-Path $MsvcupExe)) {
        throw "msvcup download failed - msvcup.exe not found at $MsvcupExe"
    }
    Write-Host "msvcup downloaded." -ForegroundColor Green
}

# --- Step 2: Install MSVC + Windows SDK via msvcup ---
Write-Host "Installing toolchain via msvcup ($MsvcVersion + $SdkVersion)..." -ForegroundColor Cyan
& $MsvcupExe install $MsvcDir --manifest-update-off autoenv $MsvcVersion $SdkVersion
if ($LASTEXITCODE -ne 0) { throw "msvcup install failed with exit code $LASTEXITCODE" }
Write-Host "Toolchain installed." -ForegroundColor Green

# --- Step 3: Set up AutoSDK junctions for UBT ---
$HostDir = Join-Path $AutoSdkDir "HostWin64\Win64"
if (-not (Test-Path $HostDir)) {
    New-Item -ItemType Directory -Path $HostDir -Force | Out-Null
}

$WinKitsJunction = Join-Path $HostDir "Windows Kits"
$VS2022Junction  = Join-Path $HostDir "VS2022"
$MsvcToolsDir    = Join-Path $MsvcDir "VC\Tools\MSVC"
$WinKitsSource   = Join-Path $MsvcDir "Windows Kits"

# Create junctions (idempotent - recreate if target changed)
foreach ($pair in @(
    @{ Link = $WinKitsJunction; Target = $WinKitsSource },
    @{ Link = $VS2022Junction;  Target = $MsvcToolsDir }
)) {
    if (Test-Path $pair.Link) {
        $existing = (Get-Item $pair.Link).Target
        if ($existing -ne $pair.Target) {
            Remove-Item $pair.Link -Force
            New-Item -ItemType Junction -Path $pair.Link -Target $pair.Target | Out-Null
        }
    } else {
        New-Item -ItemType Junction -Path $pair.Link -Target $pair.Target | Out-Null
    }
}

# --- Step 4: Set environment ---
$env:UE_SDKS_ROOT = $AutoSdkDir

# Activate vcvars for the MSVC environment (INCLUDE, LIB, PATH)
$vcvarsPath = Join-Path $MsvcDir "vcvars-x64.bat"
if (Test-Path $vcvarsPath) {
    $cmdOutput = & cmd.exe /c "call `"$vcvarsPath`" >nul 2>&1 && set"
    foreach ($line in $cmdOutput) {
        if ($line -match '^([^=]+)=(.*)$') {
            [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
        }
    }
}

# --- Step 5: Build ---
$BuildScript = Join-Path $ResolvedUnrealPath "Engine\Build\BatchFiles\Build.bat"
$ProjectFile = Join-Path $ProjectPath $ProjectName

if (-not (Test-Path $ResolvedUnrealPath)) { throw "Unreal Engine not found at '$ResolvedUnrealPath'" }
if (-not (Test-Path $BuildScript))        { throw "Build.bat not found at '$BuildScript'" }
if (-not (Test-Path $ProjectFile))        { throw "Project file not found at '$ProjectFile'" }

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "CoopGameFleep - msvcup Build"          -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Unreal Path:   $ResolvedUnrealPath"    -ForegroundColor Yellow
Write-Host "Target:        $Target"                 -ForegroundColor Yellow
Write-Host "Platform:      $Platform"               -ForegroundColor Yellow
Write-Host "Configuration: $Configuration"          -ForegroundColor Yellow
Write-Host "MSVC:          $MsvcVersion"            -ForegroundColor Yellow
Write-Host "SDK:           $SdkVersion"             -ForegroundColor Yellow

$BuildArgs = @(
    $Target,
    $Platform,
    $Configuration,
    "-Project=`"$ProjectFile`"",
    "-WaitMutex"
)

& $BuildScript $BuildArgs
$exitCode = $LASTEXITCODE

if ($exitCode -ne 0) {
    Write-Host "Build failed with exit code $exitCode." -ForegroundColor Red
    exit $exitCode
}

Write-Host "Build completed successfully." -ForegroundColor Green
exit 0
