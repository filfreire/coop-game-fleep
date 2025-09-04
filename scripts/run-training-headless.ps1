# Headless Training Launcher for CoopGameFleep
# This script launches the packaged game in headless mode for training
# Usage: .\scripts\run-training-headless.ps1 [-TrainingBuildDir "TrainingBuild"] [-MapName "P_LearningAgentsTrial"] [-LogFile "training_log.log"]

param(
    [string]$ProjectPath = (Get-Location).Path,
    [string]$TrainingBuildDir = "TrainingBuild",
    [string]$MapName = "P_LearningAgentsTrial1",  # Default learning map
    [string]$LogFile = "scharacter_training.log",
    [string]$ExeName = "CoopGameFleep.exe",
    [int]$MaxTrainingTime = 0  # 0 = unlimited, otherwise minutes
)

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "COOPGAMEFLEEP HEADLESS TRAINING" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Project Path: $ProjectPath" -ForegroundColor Yellow
Write-Host "Training Build Dir: $TrainingBuildDir" -ForegroundColor Yellow
Write-Host "Map Name: $MapName" -ForegroundColor Yellow
Write-Host "Log File: $LogFile" -ForegroundColor Yellow
Write-Host "Executable: $ExeName" -ForegroundColor Yellow

# Find the executable
$BuildPath = Join-Path $ProjectPath $TrainingBuildDir
$ExeFiles = Get-ChildItem -Path $BuildPath -Filter $ExeName -Recurse

if ($ExeFiles.Count -eq 0) {
    Write-Error "Executable '$ExeName' not found in build directory: $BuildPath"
    Write-Error "Please run package-training.ps1 first to create the training build"
    exit 1
}

$GameExecutable = $ExeFiles[0].FullName
$ExeDirectory = $ExeFiles[0].DirectoryName

Write-Host "Found executable: $GameExecutable" -ForegroundColor Green

# Change to executable directory for proper relative path resolution
Push-Location $ExeDirectory

Write-Host "`n======================================" -ForegroundColor Cyan
Write-Host "LAUNCHING HEADLESS TRAINING" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Command Line Arguments:" -ForegroundColor Yellow
Write-Host "  Map: $MapName" -ForegroundColor White
Write-Host "  Headless Training: Enabled (forces Training mode)" -ForegroundColor White
Write-Host "  Null RHI: Enabled (no rendering)" -ForegroundColor White
Write-Host "  No Sound: Enabled" -ForegroundColor White
Write-Host "  Logging: Enabled to $LogFile" -ForegroundColor White

# Build command line arguments for headless training
$GameArgs = @(
    $MapName                    # Load the training map
    "-headless-training"        # Custom flag to identify headless training mode
    "-nullrhi"                  # Disable rendering for headless mode
    "-nosound"                  # Disable sound
    "-log"                      # Enable logging to console
    "-log=$LogFile"             # Log to specific file
    "-unattended"               # Run without user interaction
    "-nothreading"              # Some training setups work better without threading
    "-NoVerifyGC"               # Skip garbage collection verification for performance
    "-NoLoadStartupPackages"    # Skip loading startup packages for faster boot
    "-FORCELOGFLUSH"            # Force log flushing for real-time monitoring
    "-ini:Engine:[Core.Log]:LogPython=Verbose"  # Enable Python logging for Learning Agents
)

# Add timeout if specified
if ($MaxTrainingTime -gt 0) {
    $GameArgs += "-ExecCmds=`"Automation RunTests;timeout $MaxTrainingTime;quit`""
    Write-Host "  Timeout: $MaxTrainingTime minutes" -ForegroundColor White
}

Write-Host "`nStarting headless training..." -ForegroundColor Green
Write-Host "Press Ctrl+C to stop training" -ForegroundColor Yellow
Write-Host "Monitor progress in: $LogFile" -ForegroundColor Cyan
Write-Host "TensorBoard logs will be in: Intermediate/LearningAgents/TensorBoard/runs" -ForegroundColor Cyan

Write-Host "`nExecuting command:" -ForegroundColor Gray
Write-Host "$ExeName $($GameArgs -join ' ')" -ForegroundColor Gray

try {
    # Start the training process (hidden window)
    $Process = Start-Process -FilePath $GameExecutable -ArgumentList $GameArgs -WindowStyle Hidden -PassThru
    
    Write-Host "`nTraining process started with PID: $($Process.Id)" -ForegroundColor Green
    Write-Host "You can monitor the log file in another terminal with:" -ForegroundColor Cyan
    Write-Host "  Get-Content -Path '$LogFile' -Wait" -ForegroundColor White
    
    # Wait for the process to complete with CTRL+C handling
    $ProcessRunning = $true
    $ExitCode = 0
    
    # Set up CTRL+C handler
    $CtrlCPressed = $false
    [Console]::CancelKeyPress += {
        param($sender, $e)
        $e.Cancel = $true  # Prevent immediate termination
        $script:CtrlCPressed = $true
        Write-Host "`n`nCTRL+C detected! Stopping training process..." -ForegroundColor Yellow
    }
    
    # Poll the process status while checking for CTRL+C
    while ($ProcessRunning -and !$CtrlCPressed) {
        if ($Process.HasExited) {
            $ProcessRunning = $false
            $ExitCode = $Process.ExitCode
        } else {
            Start-Sleep -Milliseconds 500  # Check every 500ms
        }
    }
    
    # Handle CTRL+C termination
    if ($CtrlCPressed) {
        Write-Host "Terminating training process (PID: $($Process.Id))..." -ForegroundColor Yellow
        try {
            # Try graceful termination first
            if (!$Process.HasExited) {
                $Process.CloseMainWindow()
                Start-Sleep -Seconds 2
            }
            
            # Force kill if still running
            if (!$Process.HasExited) {
                Write-Host "Force terminating training process..." -ForegroundColor Red
                $Process.Kill()
                $Process.WaitForExit(5000)  # Wait up to 5 seconds for cleanup
            }
            
            Write-Host "Training process terminated by user." -ForegroundColor Yellow
            $ExitCode = -1
        }
        catch {
            Write-Warning "Error terminating process: $($_.Exception.Message)"
        }
    }
    elseif ($ExitCode -eq 0) {
        Write-Host "`nTraining completed successfully!" -ForegroundColor Green
    } else {
        Write-Host "`nTraining ended with exit code: $ExitCode" -ForegroundColor Yellow
    }
}
catch {
    Write-Error "Failed to start training process: $($_.Exception.Message)"
    exit 1
}
finally {
    # Return to original directory
    Pop-Location
}

Write-Host "`n======================================" -ForegroundColor Cyan
Write-Host "TRAINING SESSION ENDED" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Check the following for results:" -ForegroundColor White
Write-Host "  - Log file: $ExeDirectory\$LogFile" -ForegroundColor Cyan
Write-Host "  - TensorBoard logs: $ProjectPath\Intermediate\LearningAgents\TensorBoard\runs" -ForegroundColor Cyan
Write-Host "  - Neural network snapshots in project Intermediate directory" -ForegroundColor Cyan

Write-Host "`nTo view TensorBoard, run: .\scripts\run-tensorboard.ps1" -ForegroundColor Green

# Training session completed
