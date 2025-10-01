# Batch Training Script for CoopGameFleep
# This script runs multiple headless training sessions with different random seeds
# Usage: .\scripts\run-batch-training.ps1 [-StartSeed 1] [-EndSeed 30] [-TimeoutMinutes 60] [-ConcurrentRuns 1]

param(
    [int]$StartSeed = 1,
    [int]$EndSeed = 30,
    [int]$TimeoutMinutes = 60,
    [int]$ConcurrentRuns = 1,
    [string]$ProjectPath = (Get-Location).Path,
    [string]$TrainingBuildDir = "TrainingBuild",
    [string]$MapName = "P_LearningAgentsTrial1",
    [string]$ExeName = "CoopGameFleep.exe",
    [string]$ResultsDir = "BatchTrainingResults",
    [switch]$CleanupIntermediate = $false,
    [switch]$SkipExisting = $true,
    # Obstacle configuration parameters
    [bool]$UseObstacles = $false,
    [int]$MaxObstacles = 24,
    [float]$MinObstacleSize = 60.0,
    [float]$MaxObstacleSize = 120.0,
    [string]$ObstacleMode = "Static"  # "Static" or "Dynamic"
)

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "COOPGAMEFLEEP BATCH TRAINING" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Start Seed: $StartSeed" -ForegroundColor Yellow
Write-Host "End Seed: $EndSeed" -ForegroundColor Yellow
Write-Host "Total Runs: $($EndSeed - $StartSeed + 1)" -ForegroundColor Yellow
Write-Host "Timeout per run: $TimeoutMinutes minutes" -ForegroundColor Yellow
Write-Host "Concurrent runs: $ConcurrentRuns" -ForegroundColor Yellow
Write-Host "Results directory: $ResultsDir" -ForegroundColor Yellow
Write-Host "Cleanup intermediate: $CleanupIntermediate" -ForegroundColor Yellow
Write-Host "Skip existing: $SkipExisting" -ForegroundColor Yellow
Write-Host ""
Write-Host "Obstacle Configuration:" -ForegroundColor Cyan
Write-Host "  Use Obstacles: $UseObstacles" -ForegroundColor White
Write-Host "  Max Obstacles: $MaxObstacles" -ForegroundColor White
Write-Host "  Min Obstacle Size: $MinObstacleSize" -ForegroundColor White
Write-Host "  Max Obstacle Size: $MaxObstacleSize" -ForegroundColor White
Write-Host "  Obstacle Mode: $ObstacleMode" -ForegroundColor White

# Create results directory
$ResultsPath = Join-Path $ProjectPath $ResultsDir
if (-not (Test-Path $ResultsPath)) {
    New-Item -ItemType Directory -Path $ResultsPath -Force | Out-Null
    Write-Host "Created results directory: $ResultsPath" -ForegroundColor Green
}

# Create subdirectories for organized results
$LogsDir = Join-Path $ResultsPath "Logs"
$TensorBoardDir = Join-Path $ResultsPath "TensorBoard"
$NeuralNetworksDir = Join-Path $ResultsPath "NeuralNetworks"
$SummaryDir = Join-Path $ResultsPath "Summary"

@($LogsDir, $TensorBoardDir, $NeuralNetworksDir, $SummaryDir) | ForEach-Object {
    if (-not (Test-Path $_)) {
        New-Item -ItemType Directory -Path $_ -Force | Out-Null
    }
}

Write-Host "`nResults will be organized in:" -ForegroundColor Cyan
Write-Host "  - Logs: $LogsDir" -ForegroundColor White
Write-Host "  - TensorBoard: $TensorBoardDir" -ForegroundColor White
Write-Host "  - Neural Networks: $NeuralNetworksDir" -ForegroundColor White
Write-Host "  - Summary: $SummaryDir" -ForegroundColor White

function Get-SafeTaskName {
    param([string]$Name)

    if ([string]::IsNullOrWhiteSpace($Name)) {
        return ""
    }

    $Safe = $Name.Trim()
    $Safe = $Safe -replace '[^A-Za-z0-9_\-]', '_'
    $Safe = $Safe.Trim('_')

    if ([string]::IsNullOrWhiteSpace($Safe)) {
        return ""
    }

    if ($Safe.Length -gt 60) {
        $Safe = $Safe.Substring(0, 60)
    }

    return $Safe
}

function New-UniqueTaskName {
    param([string]$BaseName = "")

    $SafeBase = Get-SafeTaskName -Name $BaseName
    if ([string]::IsNullOrWhiteSpace($SafeBase)) {
        $SafeBase = "run"
    }

    $GuidSegment = ([Guid]::NewGuid().ToString("N")).Substring(0, 8).ToLower()
    $Candidate = Get-SafeTaskName -Name "$SafeBase-$GuidSegment"

    if ([string]::IsNullOrWhiteSpace($Candidate)) {
        return $GuidSegment
    }

    return $Candidate
}

# Initialize tracking variables
$CompletedRuns = @()
$FailedRuns = @()
$SkippedRuns = @()
$StartTime = Get-Date
$RunTaskNames = @()

# Function to run a single training session
function Start-TrainingSession {
    param(
        [int]$Seed,
        [string]$SessionId,
        [string]$TrainingTaskName,
        [ref]$ResolvedTaskName
    )
    
    $LogFile = "training_seed_$Seed.log"
    $LogPath = Join-Path $LogsDir $LogFile
    $SafeTaskName = Get-SafeTaskName -Name $TrainingTaskName
    $FinalTaskName = New-UniqueTaskName -BaseName $SafeTaskName
    $ResolvedTaskName.Value = $FinalTaskName
    
    # Check if we should skip this run
    if ($SkipExisting -and (Test-Path $LogPath)) {
        Write-Host "Skipping seed $Seed - log file already exists" -ForegroundColor Yellow
        $ResolvedTaskName.Value = ""
        return "SKIPPED"
    }
    
    Write-Host "`nStarting training session $SessionId (Seed: $Seed, Task: $FinalTaskName)..." -ForegroundColor Green
    if (-not [string]::IsNullOrWhiteSpace($TrainingTaskName)) {
        Write-Host "Requested task name: $TrainingTaskName" -ForegroundColor Gray
        if ($SafeTaskName -and $SafeTaskName -ne $TrainingTaskName) {
            Write-Host "Sanitized task name: $SafeTaskName" -ForegroundColor Gray
        }
    }
    Write-Host "Resolved task name: $FinalTaskName" -ForegroundColor Gray
    Write-Host "Log file: $LogFile" -ForegroundColor Cyan
    
    try {
        # Run the training session
        $ArgumentList = @(
            "-ExecutionPolicy", "Bypass",
            "-File", "scripts/run-training-headless.ps1",
            "-RandomSeed", $Seed,
            "-LogFile", $LogFile,
            "-TimeoutMinutes", $TimeoutMinutes,
            "-UseObstacles", $UseObstacles.ToString().ToLower(),
            "-MaxObstacles", $MaxObstacles,
            "-MinObstacleSize", $MinObstacleSize,
            "-MaxObstacleSize", $MaxObstacleSize,
            "-ObstacleMode", $ObstacleMode
        )

        if ($FinalTaskName) {
            $ArgumentList += @("-TrainingTaskName", $FinalTaskName)
        }

        $Process = Start-Process -FilePath "powershell" -ArgumentList $ArgumentList -WindowStyle Hidden -PassThru -WorkingDirectory $ProjectPath
        
        Write-Host "Training process started with PID: $($Process.Id)" -ForegroundColor Green
        
        # Wait for completion with timeout - use more robust approach for SSH
        $timer = [System.Diagnostics.Stopwatch]::StartNew()
        $timeoutMs = $TimeoutMinutes * 60 * 1000
        $checkInterval = 5000  # Check every 5 seconds
        
        while (-not $Process.HasExited -and $timer.ElapsedMilliseconds -lt $timeoutMs) {
            Start-Sleep -Milliseconds $checkInterval
            
            # Check if process is still running
            try {
                $proc = Get-Process -Id $Process.Id -ErrorAction Stop
                if ($proc.HasExited) {
                    break
                }
            } catch {
                # Process no longer exists
                break
            }
        }
        
        $timer.Stop()
        
        if (-not $Process.HasExited -and $timer.ElapsedMilliseconds -ge $timeoutMs) {
            Write-Warning "Training session $SessionId timed out after $($timer.Elapsed.TotalMinutes.ToString('F1')) minutes"
            
            # Enhanced process termination - find and kill actual game processes
            Write-Host "Attempting to terminate training processes..." -ForegroundColor Yellow
            
            # First, try to find and kill the actual game executable processes
            $GameProcesses = Get-Process -Name "CoopGameFleep" -ErrorAction SilentlyContinue
            if ($GameProcesses) {
                Write-Host "Found $($GameProcesses.Count) CoopGameFleep.exe process(es)" -ForegroundColor Cyan
                foreach ($GameProc in $GameProcesses) {
                    Write-Host "Terminating CoopGameFleep.exe (PID: $($GameProc.Id)) and its process tree..." -ForegroundColor Yellow
                    
                    # Method 1: Use taskkill with /T flag to kill process tree
                    $taskkillResult = & taskkill /PID $GameProc.Id /T /F 2>&1
                    if ($LASTEXITCODE -eq 0) {
                        Write-Host "taskkill /T /F succeeded for PID $($GameProc.Id)" -ForegroundColor Green
                    } else {
                        Write-Warning "taskkill /T /F failed for PID $($GameProc.Id): $taskkillResult"
                        
                        # Method 2: Fallback - try to kill child processes manually
                        Write-Host "Attempting manual child process termination..." -ForegroundColor Yellow
                        try {
                            $childProcesses = Get-WmiObject Win32_Process | Where-Object { $_.ParentProcessId -eq $GameProc.Id }
                            foreach ($child in $childProcesses) {
                                Write-Host "Killing child process: $($child.ProcessName) (PID: $($child.ProcessId))" -ForegroundColor Gray
                                & taskkill /PID $child.ProcessId /F 2>$null
                            }
                            
                            # Now try to kill the main process again
                            & taskkill /PID $GameProc.Id /F 2>$null
                            Write-Host "Manual termination attempt completed for PID $($GameProc.Id)" -ForegroundColor Yellow
                        } catch {
                            Write-Warning "Manual child process termination failed: $($_.Exception.Message)"
                        }
                    }
                }
            } else {
                Write-Host "No CoopGameFleep.exe processes found" -ForegroundColor Yellow
            }
            
            # Also terminate the PowerShell wrapper process
            Write-Host "Terminating PowerShell wrapper process (PID: $($Process.Id))..." -ForegroundColor Yellow
            try { 
                Stop-Process -Id $Process.Id -Force -ErrorAction Stop
                Write-Host "PowerShell process terminated successfully" -ForegroundColor Green
            } catch {
                Write-Warning "Failed to terminate PowerShell process: $($_.Exception.Message)"
            }
            
            # Give processes time to terminate
            Start-Sleep -Seconds 3
            
            # Verify termination with multiple attempts
            $maxVerificationAttempts = 3
            $verificationAttempt = 0
            $allProcessesTerminated = $false
            
            while ($verificationAttempt -lt $maxVerificationAttempts -and -not $allProcessesTerminated) {
                $verificationAttempt++
                Write-Host "Verification attempt $verificationAttempt/$maxVerificationAttempts..." -ForegroundColor Cyan
                
                $RemainingGameProcesses = Get-Process -Name "CoopGameFleep" -ErrorAction SilentlyContinue
                if ($RemainingGameProcesses) {
                    Write-Warning "Warning: $($RemainingGameProcesses.Count) CoopGameFleep.exe process(es) still running"
                    
                    # Try one more aggressive termination attempt
                    foreach ($remainingProc in $RemainingGameProcesses) {
                        Write-Host "Force killing remaining process PID $($remainingProc.Id)..." -ForegroundColor Red
                        try {
                            # Try multiple termination methods
                            & taskkill /PID $remainingProc.Id /F 2>$null
                            Start-Sleep -Milliseconds 500
                            
                            # If still running, try PowerShell Stop-Process
                            $stillRunning = Get-Process -Id $remainingProc.Id -ErrorAction SilentlyContinue
                            if ($stillRunning) {
                                Stop-Process -Id $remainingProc.Id -Force -ErrorAction Stop
                            }
                        } catch {
                            Write-Warning "Failed to terminate remaining process PID $($remainingProc.Id): $($_.Exception.Message)"
                        }
                    }
                    
                    Start-Sleep -Seconds 2
                } else {
                    $allProcessesTerminated = $true
                    Write-Host "All CoopGameFleep.exe processes successfully terminated" -ForegroundColor Green
                }
            }
            
            # Final verification
            $FinalGameProcesses = Get-Process -Name "CoopGameFleep" -ErrorAction SilentlyContinue
            if ($FinalGameProcesses) {
                Write-Error "CRITICAL: $($FinalGameProcesses.Count) CoopGameFleep.exe process(es) still running after all termination attempts!"
                Write-Error "Manual intervention may be required to terminate these processes."
                foreach ($proc in $FinalGameProcesses) {
                    Write-Error "  - PID: $($proc.Id), ProcessName: $($proc.ProcessName)"
                }
            } else {
                Write-Host "SUCCESS: All CoopGameFleep.exe processes confirmed terminated" -ForegroundColor Green
            }
            
            return "TIMEOUT"
        }
        
        $ExitCode = $Process.ExitCode
        if ($ExitCode -eq 0) {
            Write-Host "Training session $SessionId completed successfully!" -ForegroundColor Green
            return "SUCCESS"
        } else {
            Write-Host "Training session $SessionId completed with exit code: $ExitCode" -ForegroundColor Yellow
            return "FAILED"
        }
        
    } catch {
        Write-Error "Failed to start training session $SessionId`: $($_.Exception.Message)"
        return "ERROR"
    }
}

# Function to copy results after training
function Copy-TrainingResults {
    param(
        [int]$Seed,
        [string]$Status,
        [string]$TrainingTaskName
    )
    
    if ($Status -eq "SUCCESS" -or $Status -eq "TIMEOUT") {
        # Copy log file
        $SourceLog = Join-Path (Join-Path $ProjectPath "TrainingBuild\Windows\CoopGameFleep\Saved\Logs") "training_seed_$Seed.log"
        $DestLog = Join-Path $LogsDir "training_seed_$Seed.log"
        
        Write-Host "Attempting to copy log file for seed $Seed..." -ForegroundColor Cyan
        Write-Host "Source: $SourceLog" -ForegroundColor Gray
        Write-Host "Destination: $DestLog" -ForegroundColor Gray
        
        if (Test-Path $SourceLog) {
            Copy-Item $SourceLog $DestLog -Force
            Write-Host "Copied log file: $SourceLog -> $DestLog" -ForegroundColor Green
        } else {
            Write-Warning "Log file not found: $SourceLog"
            # Check if the directory exists
            $LogDir = Split-Path $SourceLog -Parent
            if (Test-Path $LogDir) {
                Write-Host "Log directory exists: $LogDir" -ForegroundColor Yellow
                $AvailableLogs = Get-ChildItem $LogDir -Filter "*.log" | Select-Object Name
                if ($AvailableLogs) {
                    Write-Host "Available log files:" -ForegroundColor Yellow
                    $AvailableLogs | ForEach-Object { Write-Host "  - $($_.Name)" -ForegroundColor Gray }
                } else {
                    Write-Host "No .log files found in directory" -ForegroundColor Yellow
                }
            } else {
                Write-Host "Log directory does not exist: $LogDir" -ForegroundColor Red
            }
        }
        
        # Copy TensorBoard runs
        $SafeTaskName = Get-SafeTaskName -Name $TrainingTaskName
        $LearningAgentsRoot = Join-Path $ProjectPath "Intermediate\LearningAgents"
        $TaskFolder = $null

        if (Test-Path $LearningAgentsRoot) {
            $AllCandidates = Get-ChildItem -Path $LearningAgentsRoot -Directory -ErrorAction SilentlyContinue
            if ($AllCandidates) {
                if ($SafeTaskName) {
                    $pattern = "$SafeTaskName*"
                    $Matched = $AllCandidates | Where-Object { $_.Name -like $pattern }
                    if (-not $Matched) {
                        Write-Warning "No Learning Agents directories matched pattern '$pattern'. Falling back to latest entry."
                        $Matched = $AllCandidates
                    }
                } else {
                    $Matched = $AllCandidates | Where-Object { $_.Name -like "Training*" }
                    if (-not $Matched) {
                        $Matched = $AllCandidates
                    }
                }

                $TaskFolder = $Matched | Sort-Object LastWriteTime -Descending | Select-Object -First 1
            }
        }

        if ($TaskFolder) {
            Write-Host "Using Learning Agents folder: $($TaskFolder.FullName)" -ForegroundColor Gray

            $TensorBoardSource = Join-Path $TaskFolder.FullName "TensorBoard\runs"
            if (Test-Path $TensorBoardSource) {
                $LatestRun = Get-ChildItem $TensorBoardSource -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
                if ($LatestRun) {
                    $TensorBoardDest = Join-Path $TensorBoardDir "seed_$Seed"
                    Copy-Item $LatestRun.FullName $TensorBoardDest -Recurse -Force
                }
            } else {
                Write-Warning "TensorBoard runs directory not found for seed $Seed"
            }

            $NeuralNetSource = Join-Path $TaskFolder.FullName "NeuralNetworks"
            if (Test-Path $NeuralNetSource) {
                $NeuralNetDest = Join-Path $NeuralNetworksDir "seed_$Seed"
                Copy-Item $NeuralNetSource $NeuralNetDest -Recurse -Force
            } else {
                Write-Warning "Neural network directory not found for seed $Seed"
            }

            if ($CleanupIntermediate) {
                if (Test-Path $TensorBoardSource) {
                    Remove-Item $TensorBoardSource -Recurse -Force -ErrorAction SilentlyContinue
                }
                if (Test-Path $NeuralNetSource) {
                    Remove-Item $NeuralNetSource -Recurse -Force -ErrorAction SilentlyContinue
                }

                try {
                    $remainingItems = Get-ChildItem -Path $TaskFolder.FullName -Force -ErrorAction SilentlyContinue
                    if (-not $remainingItems) {
                        Remove-Item $TaskFolder.FullName -Force -ErrorAction SilentlyContinue
                    }
                } catch {
                    Write-Warning "Failed to clean up task folder '$($TaskFolder.FullName)': $($_.Exception.Message)"
                }
            }
        } else {
            Write-Warning "No Learning Agents output directory found for seed $Seed"
        }
    }
}

# Main training loop
Write-Host "`n======================================" -ForegroundColor Cyan
Write-Host "STARTING BATCH TRAINING" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan

$CurrentRun = 1
$TotalRuns = $EndSeed - $StartSeed + 1

for ($Seed = $StartSeed; $Seed -le $EndSeed; $Seed++) {
    $SessionId = "Run $CurrentRun/$TotalRuns"
    Write-Host "`n[$SessionId] Processing seed $Seed..." -ForegroundColor Cyan
    
    $RequestedTaskName = "seed_$Seed"
    $ResolvedTaskName = ""

    $Status = Start-TrainingSession -Seed $Seed -SessionId $SessionId -TrainingTaskName $RequestedTaskName -ResolvedTaskName ([ref]$ResolvedTaskName)
    
    # Track results
    switch ($Status) {
        "SUCCESS" { $CompletedRuns += $Seed }
        "TIMEOUT" { $CompletedRuns += $Seed }  # Timeouts are successful if they complete training
        "FAILED" { $FailedRuns += $Seed }
        "ERROR" { $FailedRuns += $Seed }
        "SKIPPED" { $SkippedRuns += $Seed }
    }
    
    # Copy results
    Copy-TrainingResults -Seed $Seed -Status $Status -TrainingTaskName $ResolvedTaskName

    if (-not [string]::IsNullOrWhiteSpace($ResolvedTaskName)) {
        $RunTaskNames += [PSCustomObject]@{
            Seed = $Seed
            TaskName = $ResolvedTaskName
        }
    }
    
    $CurrentRun++
    
    # Add delay between runs to avoid resource conflicts
    if ($CurrentRun -le $TotalRuns) {
        Write-Host "Waiting 30 seconds before next run..." -ForegroundColor Gray
        Start-Sleep -Seconds 30
    }
}

# Generate summary report
$EndTime = Get-Date
$TotalDuration = $EndTime - $StartTime

$TaskNameSummary = if ($RunTaskNames.Count -gt 0) {
    ($RunTaskNames | ForEach-Object { "  Seed $($_.Seed): $($_.TaskName)" }) -join "`n"
} else {
    "  (none recorded)"
}

$SummaryReport = @"
BATCH TRAINING SUMMARY REPORT
=============================
Start Time: $($StartTime.ToString("yyyy-MM-dd HH:mm:ss"))
End Time: $($EndTime.ToString("yyyy-MM-dd HH:mm:ss"))
Total Duration: $($TotalDuration.ToString("hh\:mm\:ss"))

CONFIGURATION:
- Seed Range: $StartSeed to $EndSeed
- Total Runs: $TotalRuns
- Timeout per run: $TimeoutMinutes minutes
- Concurrent runs: $ConcurrentRuns

RESULTS:
- Successful runs: $($CompletedRuns.Count) - Seeds: $($CompletedRuns -join ', ')
- Failed runs: $($FailedRuns.Count) - Seeds: $($FailedRuns -join ', ')
- Skipped runs: $($SkippedRuns.Count) - Seeds: $($SkippedRuns -join ', ')

TASK NAMES:
$TaskNameSummary

FILES GENERATED:
- Log files: $LogsDir
- TensorBoard runs: $TensorBoardDir
- Neural network files: $NeuralNetworksDir
- This summary: $SummaryDir

NEXT STEPS:
1. Review individual log files for detailed training progress
2. Use TensorBoard to visualize training metrics: .\scripts\run-tensorboard.ps1 --log-dir "$TensorBoardDir"
3. Compare neural network performance across different seeds
4. Analyze results to determine optimal hyperparameters
"@

$SummaryFile = Join-Path $SummaryDir "batch_training_summary_$(Get-Date -Format 'yyyy-MM-dd_HH-mm-ss').txt"
$SummaryReport | Out-File -FilePath $SummaryFile -Encoding UTF8

Write-Host "`n======================================" -ForegroundColor Cyan
Write-Host "BATCH TRAINING COMPLETED" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Total Duration: $($TotalDuration.ToString("hh\:mm\:ss"))" -ForegroundColor Yellow
Write-Host "Successful runs: $($CompletedRuns.Count)/$TotalRuns" -ForegroundColor Green
Write-Host "Failed runs: $($FailedRuns.Count)/$TotalRuns" -ForegroundColor Red
Write-Host "Skipped runs: $($SkippedRuns.Count)/$TotalRuns" -ForegroundColor Yellow

Write-Host "`nResults saved to: $ResultsPath" -ForegroundColor Cyan
Write-Host "Summary report: $SummaryFile" -ForegroundColor Cyan

Write-Host "`nTo view TensorBoard for all runs:" -ForegroundColor Green
Write-Host ".\scripts\run-tensorboard.ps1 --log-dir `"$TensorBoardDir`"" -ForegroundColor White

# Batch training completed
