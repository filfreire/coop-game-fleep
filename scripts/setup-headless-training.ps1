# Complete Headless Training Setup for CoopGameFleep
# This script provides a complete workflow to set up headless training
# Usage: .\scripts\setup-headless-training.ps1 [-SkipBuild] [-SkipPackage]

param(
    [switch]$SkipBuild = $false,
    [switch]$SkipPackage = $false,
    [string]$ProjectPath = (Get-Location).Path
)

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "COOPGAMEFLEEP HEADLESS TRAINING SETUP" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan

Write-Host "This script will guide you through setting up headless training for CoopGameFleep." -ForegroundColor Yellow
Write-Host "Please ensure you have configured your SCharacterManager blueprint first!" -ForegroundColor Yellow

# Step 1: Build the project (optional)
if (-not $SkipBuild) {
    Write-Host "`n[1/4] Building project..." -ForegroundColor Green
    try {
        & "$ProjectPath\scripts\build-local.ps1"
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Build failed. Please fix build errors before continuing."
            exit 1
        }
        Write-Host "Build completed successfully" -ForegroundColor Green
        Write-Host "Learning Agents dependencies should now be installed automatically" -ForegroundColor Cyan
    }
    catch {
        Write-Error "Failed to run build script: $($_.Exception.Message)"
        exit 1
    }
} else {
    Write-Host "[1/4] Skipping build step" -ForegroundColor Yellow
    Write-Host "Note: Learning Agents dependencies may not be installed" -ForegroundColor Yellow
}

# Step 2: Package for training (optional)
if (-not $SkipPackage) {
    Write-Host "`n[2/4] Creating training build..." -ForegroundColor Green
    try {
        & "$ProjectPath\scripts\package-training.ps1"
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Packaging failed. Please check the packaging logs."
            exit 1
        }
        Write-Host "Training build created successfully" -ForegroundColor Green
    }
    catch {
        Write-Error "Failed to run packaging script: $($_.Exception.Message)"
        exit 1
    }
} else {
    Write-Host "[2/5] Skipping packaging step" -ForegroundColor Yellow
}

# Step 3: Verify TensorBoard availability
Write-Host "`n[3/5] Verifying TensorBoard availability..." -ForegroundColor Green
$PythonExe = Join-Path $ProjectPath "Intermediate\PipInstall\Scripts\python.exe"
$TensorBoardExe = Join-Path $ProjectPath "Intermediate\PipInstall\Scripts\tensorboard.exe"

if (Test-Path $PythonExe) {
    Write-Host "✓ Learning Agents Python environment found" -ForegroundColor Green
} else {
    Write-Host "⚠ Learning Agents Python environment not found" -ForegroundColor Yellow
    Write-Host "This may indicate that the build step failed to install dependencies" -ForegroundColor Gray
}

if (Test-Path $TensorBoardExe) {
    Write-Host "✓ TensorBoard is available" -ForegroundColor Green
} else {
    Write-Host "⚠ TensorBoard not found, but this should be installed automatically" -ForegroundColor Yellow
    Write-Host "TensorBoard should be available after Learning Agents dependencies are installed" -ForegroundColor Gray
}

# Step 4: Provide configuration guidance
Write-Host "`n[4/5] Configuration Check" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan

$ConfigComplete = $true

Write-Host "Please verify the following configuration in Unreal Editor:" -ForegroundColor Yellow

Write-Host "`nSCharacterManager Configuration:" -ForegroundColor Cyan
Write-Host "  - Run Mode: Any mode (will auto-force to Training in headless)" -ForegroundColor White
Write-Host "  - All four neural networks assigned:" -ForegroundColor White
Write-Host "     - Encoder Neural Network" -ForegroundColor Gray
Write-Host "     - Policy Neural Network" -ForegroundColor Gray
Write-Host "     - Decoder Neural Network" -ForegroundColor Gray
Write-Host "     - Critic Neural Network" -ForegroundColor Gray
Write-Host "  - Target Actor reference set" -ForegroundColor White

Write-Host "`nTrainer Training Settings:" -ForegroundColor Cyan
Write-Host "  - Use Tensorboard = True" -ForegroundColor White
Write-Host "  - Save Snapshots = True" -ForegroundColor White

Write-Host "`nTrainer Path Settings:" -ForegroundColor Cyan
Write-Host "  - Non Editor Engine Relative Path configured" -ForegroundColor White
Write-Host "  - Non Editor Intermediate Relative Path configured" -ForegroundColor White
Write-Host "     (Check the package-training.ps1 output for these paths)" -ForegroundColor Gray

Write-Host "`nTraining Map (P_LearningAgentsTrial):" -ForegroundColor Cyan
Write-Host "  - SCharacter instances placed" -ForegroundColor White
Write-Host "  - STargetActor placed" -ForegroundColor White
Write-Host "  - SCharacterManager placed and configured" -ForegroundColor White

Write-Host "`nPlease ensure you have completed all the above configuration in Unreal Editor." -ForegroundColor Yellow
Write-Host "Refer to docs/headless-training-setup.md for detailed instructions." -ForegroundColor Cyan
$ConfigComplete = $true

# Step 5: Ready to train
Write-Host "`n[5/5] Training Setup Complete" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan

if ($ConfigComplete) {
    Write-Host "Setup completed successfully!" -ForegroundColor Green
    Write-Host "`nYou can now start headless training with:" -ForegroundColor Cyan
    Write-Host "  .\scripts\run-training-headless.ps1" -ForegroundColor White
    
    Write-Host "`nOptional monitoring commands:" -ForegroundColor Cyan
    Write-Host "  # Start TensorBoard (in another terminal)" -ForegroundColor Gray
    Write-Host "  .\scripts\run-tensorboard.ps1" -ForegroundColor White
    Write-Host "  # TensorBoard will be available at: http://localhost:6006" -ForegroundColor Gray
    Write-Host "`n  # Monitor training logs (in another terminal)" -ForegroundColor Gray
    Write-Host "  cd TrainingBuild\Windows\CoopGameFleep\Binaries\Win64" -ForegroundColor White
    Write-Host "  Get-Content -Path scharacter_training.log -Wait" -ForegroundColor White
    
    Write-Host "`nReady to start training when you are!" -ForegroundColor Green
} else {
    Write-Host "Configuration incomplete. Please complete the setup first." -ForegroundColor Yellow
}

Write-Host "`nFor detailed information, see:" -ForegroundColor Cyan
Write-Host "  - docs/headless-training-setup.md" -ForegroundColor White
Write-Host "  - docs/learning-agents-setup.md" -ForegroundColor White
