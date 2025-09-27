# Example script showing how to run training with obstacle configuration
# This demonstrates the new command-line obstacle parameters

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "COOPGAMEFLEEP TRAINING WITH OBSTACLES" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan

# Example 1: Static obstacles (default)
Write-Host "`nExample 1: Static Obstacles (24 obstacles, 60-120 size - Wall-like)" -ForegroundColor Yellow
& ".\scripts\run-training-headless.ps1" -UseObstacles $true -MaxObstacles 24 -MinObstacleSize 60 -MaxObstacleSize 120 -ObstacleMode "Static"

# Example 2: Dynamic obstacles (regenerated each episode)
Write-Host "`nExample 2: Dynamic Obstacles (30 obstacles, 50-150 size - Wall-like)" -ForegroundColor Yellow
& ".\scripts\run-training-headless.ps1" -UseObstacles $true -MaxObstacles 30 -MinObstacleSize 50 -MaxObstacleSize 150 -ObstacleMode "Dynamic"

# Example 3: No obstacles (for comparison)
Write-Host "`nExample 3: No Obstacles (for baseline comparison)" -ForegroundColor Yellow
& ".\scripts\run-training-headless.ps1" -UseObstacles $false

# Example 4: Many small obstacles
Write-Host "`nExample 4: Many Small Obstacles (40 obstacles, 40-100 size - Wall-like)" -ForegroundColor Yellow
& ".\scripts\run-training-headless.ps1" -UseObstacles $true -MaxObstacles 40 -MinObstacleSize 40 -MaxObstacleSize 100 -ObstacleMode "Static"

Write-Host "`n======================================" -ForegroundColor Cyan
Write-Host "OBSTACLE CONFIGURATION EXAMPLES COMPLETE" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "`nAvailable Parameters:" -ForegroundColor White
Write-Host "  -UseObstacles: true/false (enable/disable obstacles)" -ForegroundColor Gray
Write-Host "  -MaxObstacles: number (maximum obstacles to spawn)" -ForegroundColor Gray
Write-Host "  -MinObstacleSize: float (minimum obstacle size)" -ForegroundColor Gray
Write-Host "  -MaxObstacleSize: float (maximum obstacle size)" -ForegroundColor Gray
Write-Host "  -ObstacleMode: Static/Dynamic (obstacle behavior)" -ForegroundColor Gray
Write-Host "`nStatic Mode: Obstacles placed once, same positions throughout training" -ForegroundColor Cyan
Write-Host "Dynamic Mode: Obstacles regenerated each training episode" -ForegroundColor Cyan
