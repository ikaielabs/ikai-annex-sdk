# Get the directory containing this script

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# The SDK root is the parent of the scripts directory

$ikainexRoot = Split-Path -Parent $scriptDir

# Store as a persistent User Environment Variable

[Environment]::SetEnvironmentVariable(
"IKAINEX_ROOT",
$ikainexRoot,
"User"
)

Write-Host ""
Write-Host "=========================================="
Write-Host "IKAINEX_ROOT configured successfully"
Write-Host "=========================================="
Write-Host $ikainexRoot
Write-Host ""
Write-Host "Please restart STM32CubeIDE if it is open."
Write-Host ""
