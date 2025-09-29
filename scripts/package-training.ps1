# Package script for CoopGameFleep project - TRAINING BUILD
# This creates a Development build suitable for headless training with Learning Agents
# Usage: .\scripts\package-training.ps1 [-Target "CoopGameFleep"] [-Platform "Win64"] [-OutputDir "TrainingBuild"]

param(
    [string]$UnrealPath = "",
    [string]$ProjectPath = (Get-Location).Path,
    [string]$ProjectName = "CoopGameFleep.uproject",
    [string]$Target = "CoopGameFleep",
    [string]$Platform = "Win64",
    [string]$Config = "Development",  # Development for training, not Shipping
    [string]$OutputDir = "TrainingBuild"
)

# Helper function to calculate relative path (compatible with older PowerShell versions)
function Get-RelativePath {
    param([string]$FromPath, [string]$ToPath)

    $FromPathNormalized = (Resolve-Path $FromPath).Path.TrimEnd('\')
    $ToPathNormalized = (Resolve-Path $ToPath).Path.TrimEnd('\')

    # Split paths into components
    $FromParts = $FromPathNormalized.Split('\')
    $ToParts = $ToPathNormalized.Split('\')

    # Find common root
    $CommonLength = 0
    $MinLength = [Math]::Min($FromParts.Length, $ToParts.Length)

    for ($i = 0; $i -lt $MinLength; $i++) {
        if ($FromParts[$i] -eq $ToParts[$i]) {
            $CommonLength++
        } else {
            break
        }
    }

    # Calculate relative path
    $UpLevels = $FromParts.Length - $CommonLength
    $RelativeParts = @()

    # Add ".." for each level up
    for ($i = 0; $i -lt $UpLevels; $i++) {
        $RelativeParts += ".."
    }

    # Add remaining path components
    for ($i = $CommonLength; $i -lt $ToParts.Length; $i++) {
        $RelativeParts += $ToParts[$i]
    }

    $RelativePath = $RelativeParts -join '/'
    return $RelativePath
}

# Helper function to calculate relative path to Engine
function Get-RelativePathToEngine {
    param([string]$ExePath, [string]$UnrealPath)

    $UnrealEngineDir = Join-Path $UnrealPath "Engine"
    return Get-RelativePath -FromPath $ExePath -ToPath $UnrealEngineDir
}

# Helper function to calculate relative path to Intermediate
function Get-RelativePathToIntermediate {
    param([string]$ExePath, [string]$ProjectPath)

    $IntermediateDir = Join-Path $ProjectPath "Intermediate"
    return Get-RelativePath -FromPath $ExePath -ToPath $IntermediateDir
}

# Determine UnrealPath based on hostname if not provided
if ([string]::IsNullOrEmpty($UnrealPath)) {
    $hostname = [System.Net.Dns]::GetHostName()
    if ($hostname -eq "filfreire01") {
        $UnrealPath = "C:\unreal\UE_5.6"
    } elseif ($hostname -eq "filfreire02") {
        $UnrealPath = "D:\unreal\UE_5.6"
    } elseif ($hostname -eq "desktop-doap6m9") {
        $UnrealPath = "E:\unreal\UE_5.6"
    } elseif ($hostname -like "unreal-*") {
        $UnrealPath = "c:\unreal\UE_5.6"
    } else {
        # Default path if hostname doesn't match known patterns
        $UnrealPath = "D:\unreal\UE_5.6"
    }
}

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Packaging CoopGameFleep TRAINING BUILD" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Unreal Path: $UnrealPath" -ForegroundColor Yellow
Write-Host "Project Path: $ProjectPath" -ForegroundColor Yellow
Write-Host "Project Name: $ProjectName" -ForegroundColor Yellow
Write-Host "Target: $Target" -ForegroundColor Yellow
Write-Host "Platform: $Platform" -ForegroundColor Yellow
Write-Host "Configuration: $Config" -ForegroundColor Yellow
Write-Host "Output Directory: $OutputDir" -ForegroundColor Yellow

$RunUATScript = Join-Path $UnrealPath "Engine\Build\BatchFiles\RunUAT.bat"
$ProjectFile = Join-Path $ProjectPath $ProjectName
$PackageFolder = Join-Path $ProjectPath $OutputDir

if (-not (Test-Path $RunUATScript)) {
    Write-Error "RunUAT script not found at: $RunUATScript"
    Write-Error "Please check your Unreal Engine installation path"
    exit 1
}

if (-not (Test-Path $ProjectFile)) {
    Write-Error "Project file not found at: $ProjectFile"
    Write-Error "Please check your project path and name"
    exit 1
}

# Create output directory if it doesn't exist
if (-not (Test-Path $PackageFolder)) {
    New-Item -ItemType Directory -Path $PackageFolder -Force
    Write-Host "Created output directory: $PackageFolder" -ForegroundColor Cyan
}

Write-Host "Starting packaging process for training..." -ForegroundColor Cyan
Write-Host "This may take several minutes..." -ForegroundColor Yellow

# Build the RunUAT command arguments for training build
$UATArgs = @(
    "BuildCookRun"
    "-project=`"$ProjectFile`""
    "-nop4"
    "-utf8output"
    "-nocompileeditor"
    "-skipbuildeditor"
    "-cook"
    "-project=`"$ProjectFile`""
    "-target=$Target"
    "-platform=$Platform"
    "-installed"
    "-stage"
    "-archive"
    "-package"
    "-build"
    "-pak"
    "-compressed"
    "-archivedirectory=`"$PackageFolder`""
    "-clientconfig=$Config"
    "-nocompile"
    "-nocompileuat"
    # Keep debug info for training builds
    # "-nodebuginfo"  # Commented out for training
)

# Execute the packaging command
Write-Host "Executing: $RunUATScript $($UATArgs -join ' ')" -ForegroundColor Gray
& $RunUATScript $UATArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host "======================================" -ForegroundColor Green
    Write-Host "PACKAGING COMPLETED SUCCESSFULLY!" -ForegroundColor Green
    Write-Host "======================================" -ForegroundColor Green
    Write-Host "Training build location: $PackageFolder" -ForegroundColor Cyan

    # Install Learning Agents dependencies for headless training
    Write-Host "`nInstalling Learning Agents dependencies..." -ForegroundColor Yellow
    try {
        & "$ProjectPath\scripts\install-learning-agents-deps.ps1" -UnrealPath $UnrealPath -ProjectPath $ProjectPath
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Learning Agents dependencies installed successfully!" -ForegroundColor Green
        } else {
            Write-Warning "Learning Agents dependency installation failed, but packaging completed"
        }
    } catch {
        Write-Warning "Failed to install Learning Agents dependencies: $_"
    }

    # Try to find the executable
    $ExeFiles = Get-ChildItem -Path $PackageFolder -Filter "*.exe" -Recurse
    if ($ExeFiles.Count -gt 0) {
        Write-Host "`nGame executable(s) found:" -ForegroundColor Green
        foreach ($exe in $ExeFiles) {
            Write-Host "  $($exe.FullName)" -ForegroundColor White
        }

        # Calculate relative paths for Non Editor settings
        $FirstExe = $ExeFiles[0]
        $ExeDir = $FirstExe.DirectoryName
        Write-Host "`n======================================" -ForegroundColor Cyan
        Write-Host "NON EDITOR PATH CONFIGURATION" -ForegroundColor Yellow
        Write-Host "======================================" -ForegroundColor Cyan
        Write-Host "For your Learning Manager settings, use these relative paths:" -ForegroundColor Yellow

        # Calculate relative path to Engine
        $RelativeToEngine = Get-RelativePathToEngine -ExePath $ExeDir -UnrealPath $UnrealPath
        Write-Host "`nNon Editor Engine Relative Path:" -ForegroundColor Green
        Write-Host "  $RelativeToEngine" -ForegroundColor White

        # Calculate relative path to Intermediate
        $RelativeToIntermediate = Get-RelativePathToIntermediate -ExePath $ExeDir -ProjectPath $ProjectPath
        Write-Host "`nNon Editor Intermediate Relative Path:" -ForegroundColor Green
        Write-Host "  $RelativeToIntermediate" -ForegroundColor White

        Write-Host "`n======================================" -ForegroundColor Cyan
        Write-Host "NEXT STEPS FOR HEADLESS TRAINING:" -ForegroundColor Yellow
        Write-Host "======================================" -ForegroundColor Cyan
        Write-Host "1. Open the SCharacterManager blueprint" -ForegroundColor White
        Write-Host "2. Set Run Mode to 'Training'" -ForegroundColor White
        Write-Host "3. In Trainer Training Settings:" -ForegroundColor White
        Write-Host "   - Enable Use Tensorboard = True" -ForegroundColor White
        Write-Host "   - Enable Save Snapshots = True" -ForegroundColor White
        Write-Host "4. In Trainer Path Settings, set the paths above" -ForegroundColor White
        Write-Host "5. Use the run-training-headless.ps1 script to start training" -ForegroundColor White
    }
} else {
    Write-Error "Packaging failed with exit code: $LASTEXITCODE"
    exit $LASTEXITCODE
}
