# Emergency Shutdown Script for CoopGameFleep
# This script forcefully terminates all running CoopGameFleep processes
# Usage: .\scripts\emergency-shutdown.ps1

param(
    [switch]$Force = $false,
    [switch]$Verbose = $false
)

Write-Host "======================================" -ForegroundColor Red
Write-Host "EMERGENCY SHUTDOWN - COOPGAMEFLEEP" -ForegroundColor Red
Write-Host "======================================" -ForegroundColor Red
Write-Host ""

# Set the project directory
$ProjectDir = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectDir

Write-Host "Project Directory: $ProjectDir" -ForegroundColor Yellow
Write-Host "Timestamp: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Yellow
Write-Host ""

# Function to kill processes with multiple methods
function Stop-ProcessForcefully {
    param(
        [string]$ProcessName,
        [string]$Description
    )
    
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "TERMINATING $Description" -ForegroundColor Yellow
    Write-Host "======================================" -ForegroundColor Cyan
    
    # Find all processes with the given name
    $Processes = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue
    
    if (-not $Processes) {
        Write-Host "No $ProcessName processes found" -ForegroundColor Green
        return $true
    }
    
    Write-Host "Found $($Processes.Count) $ProcessName process(es)" -ForegroundColor Yellow
    
    $AllTerminated = $true
    
    foreach ($Process in $Processes) {
        Write-Host "Terminating $ProcessName (PID: $($Process.Id))..." -ForegroundColor Yellow
        
        # Method 1: Try graceful termination first (unless Force is specified)
        if (-not $Force) {
            try {
                $Process.CloseMainWindow()
                Start-Sleep -Seconds 2
                
                # Check if process is still running
                $StillRunning = Get-Process -Id $Process.Id -ErrorAction SilentlyContinue
                if (-not $StillRunning) {
                    Write-Host "  Graceful termination successful for PID $($Process.Id)" -ForegroundColor Green
                    continue
                }
            } catch {
                Write-Host "  Graceful termination failed for PID $($Process.Id): $($_.Exception.Message)" -ForegroundColor Yellow
            }
        }
        
        # Method 2: Use taskkill with /T flag to kill process tree
        Write-Host "  Using taskkill /T /F for PID $($Process.Id)..." -ForegroundColor Yellow
        $taskkillResult = & taskkill /PID $Process.Id /T /F 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  taskkill /T /F succeeded for PID $($Process.Id)" -ForegroundColor Green
        } else {
            Write-Warning "  taskkill /T /F failed for PID $($Process.Id): $taskkillResult"
            $AllTerminated = $false
            
            # Method 3: Try to kill child processes manually
            Write-Host "  Attempting manual child process termination..." -ForegroundColor Yellow
            try {
                $childProcesses = Get-WmiObject Win32_Process | Where-Object { $_.ParentProcessId -eq $Process.Id }
                foreach ($child in $childProcesses) {
                    Write-Host "    Killing child process: $($child.ProcessName) (PID: $($child.ProcessId))" -ForegroundColor Gray
                    & taskkill /PID $child.ProcessId /F 2>$null
                }
                
                # Now try to kill the main process again
                & taskkill /PID $Process.Id /F 2>$null
                Write-Host "  Manual termination attempt completed for PID $($Process.Id)" -ForegroundColor Yellow
            } catch {
                Write-Warning "  Manual child process termination failed: $($_.Exception.Message)"
            }
        }
    }
    
    # Give processes time to terminate
    Start-Sleep -Seconds 3
    
    # Verify termination with multiple attempts
    $maxVerificationAttempts = 5
    $verificationAttempt = 0
    
    while ($verificationAttempt -lt $maxVerificationAttempts) {
        $verificationAttempt++
        Write-Host "  Verification attempt $verificationAttempt/$maxVerificationAttempts..." -ForegroundColor Cyan
        
        $RemainingProcesses = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue
        if ($RemainingProcesses) {
            Write-Warning "  Warning: $($RemainingProcesses.Count) $ProcessName process(es) still running"
            
            # Try more aggressive termination attempts
            foreach ($remainingProc in $RemainingProcesses) {
                Write-Host "    Force killing remaining process PID $($remainingProc.Id)..." -ForegroundColor Red
                try {
                    # Try multiple termination methods
                    & taskkill /PID $remainingProc.Id /F 2>$null
                    Start-Sleep -Milliseconds 500
                    
                    # If still running, try PowerShell Stop-Process
                    $stillRunning = Get-Process -Id $remainingProc.Id -ErrorAction SilentlyContinue
                    if ($stillRunning) {
                        Stop-Process -Id $remainingProc.Id -Force -ErrorAction Stop
                    }
                    
                    # If still running, try wmic
                    $stillRunning = Get-Process -Id $remainingProc.Id -ErrorAction SilentlyContinue
                    if ($stillRunning) {
                        & wmic process where "ProcessId=$($remainingProc.Id)" delete 2>$null
                    }
                } catch {
                    Write-Warning "    Failed to terminate remaining process PID $($remainingProc.Id): $($_.Exception.Message)"
                }
            }
            
            Start-Sleep -Seconds 2
        } else {
            Write-Host "  All $ProcessName processes successfully terminated" -ForegroundColor Green
            $AllTerminated = $true
            break
        }
    }
    
    # Final verification
    $FinalProcesses = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue
    if ($FinalProcesses) {
        Write-Error "  CRITICAL: $($FinalProcesses.Count) $ProcessName process(es) still running after all termination attempts!"
        Write-Error "  Manual intervention may be required to terminate these processes."
        foreach ($proc in $FinalProcesses) {
            Write-Error "    - PID: $($proc.Id), ProcessName: $($proc.ProcessName)"
        }
        $AllTerminated = $false
    } else {
        Write-Host "  SUCCESS: All $ProcessName processes confirmed terminated" -ForegroundColor Green
    }
    
    Write-Host ""
    return $AllTerminated
}

# Function to kill related processes
function Stop-RelatedProcesses {
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "TERMINATING RELATED PROCESSES" -ForegroundColor Yellow
    Write-Host "======================================" -ForegroundColor Cyan
    
    # List of related process names that might be running
    $RelatedProcesses = @(
        "UnrealEditor-CoopGameFleep",
        "CoopGameFleepEditor",
        "UE4Editor",
        "UE5Editor"
    )
    
    $AllRelatedTerminated = $true
    
    foreach ($ProcessName in $RelatedProcesses) {
        $Processes = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue
        if ($Processes) {
            Write-Host "Found $($Processes.Count) $ProcessName process(es)" -ForegroundColor Yellow
            $Terminated = Stop-ProcessForcefully -ProcessName $ProcessName -Description $ProcessName
            if (-not $Terminated) {
                $AllRelatedTerminated = $false
            }
        }
    }
    
    return $AllRelatedTerminated
}

# Main execution
Write-Host "Starting emergency shutdown procedure..." -ForegroundColor Red
Write-Host ""

# Step 1: Kill main CoopGameFleep processes
$MainTerminated = Stop-ProcessForcefully -ProcessName "CoopGameFleep" -Description "MAIN COOPGAMEFLEEP PROCESSES"

# Step 2: Kill related processes
$RelatedTerminated = Stop-RelatedProcesses

# Step 3: Additional cleanup - kill any processes that might be holding onto game resources
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "ADDITIONAL CLEANUP" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan

# Kill any processes that might be using the game's log files or resources
$LogProcesses = Get-Process | Where-Object { 
    $_.ProcessName -like "*CoopGame*" -or 
    $_.ProcessName -like "*Fleep*" -or
    $_.MainWindowTitle -like "*CoopGame*" -or
    $_.MainWindowTitle -like "*Fleep*"
} -ErrorAction SilentlyContinue

if ($LogProcesses) {
    Write-Host "Found additional related processes:" -ForegroundColor Yellow
    foreach ($proc in $LogProcesses) {
        Write-Host "  - $($proc.ProcessName) (PID: $($proc.Id))" -ForegroundColor Gray
        try {
            Stop-Process -Id $proc.Id -Force -ErrorAction Stop
            Write-Host "    Terminated successfully" -ForegroundColor Green
        } catch {
            Write-Warning "    Failed to terminate: $($_.Exception.Message)"
        }
    }
} else {
    Write-Host "No additional related processes found" -ForegroundColor Green
}

Write-Host ""

# Final status report
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "EMERGENCY SHUTDOWN COMPLETE" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan

$OverallSuccess = $MainTerminated -and $RelatedTerminated

if ($OverallSuccess) {
    Write-Host "SUCCESS: All CoopGameFleep processes have been terminated" -ForegroundColor Green
    Write-Host ""
    Write-Host "You can now safely:" -ForegroundColor White
    Write-Host "  - Start new training runs" -ForegroundColor Cyan
    Write-Host "  - Restart the batch training script" -ForegroundColor Cyan
    Write-Host "  - Check system resources" -ForegroundColor Cyan
} else {
    Write-Host "WARNING: Some processes may still be running" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "If you continue to have issues:" -ForegroundColor White
    Write-Host "  - Check Task Manager for remaining processes" -ForegroundColor Cyan
    Write-Host "  - Restart your computer if necessary" -ForegroundColor Cyan
    Write-Host "  - Check for hung processes in Resource Monitor" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "Emergency shutdown completed at: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Yellow

# Exit with appropriate code
if ($OverallSuccess) {
    exit 0
} else {
    exit 1
}
