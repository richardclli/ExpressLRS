#Requires -Version 7
[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [ValidateSet('EL18', 'FRM303', 'PL18U')]
    [string]$Target = 'EL18',

    [ValidateSet('FlyskyBL', 'STLINK')]
    [string]$FlashMethod = 'FlyskyBL',

    [switch]$Clean,

    [switch]$Open
)

$ErrorActionPreference = 'Stop'
$repoRoot = $PSScriptRoot
$pio = Join-Path $repoRoot '.venv\Scripts\pio.exe'
$projectDir = Join-Path $repoRoot 'src'

if (-not (Test-Path $pio)) {
    Write-Host "PlatformIO venv not found at $pio" -ForegroundColor Red
    Write-Host "Bootstrap with:" -ForegroundColor Yellow
    Write-Host "  python -m venv .venv"
    Write-Host "  .\.venv\Scripts\python.exe -m pip install platformio 'esptool<5'"
    exit 1
}

$verb = if ($FlashMethod -eq 'STLINK') { 'via' } else { 'for' }
$envName = "Flysky_${Target}_TX_${verb}_${FlashMethod}"

Write-Host "Building $envName ..." -ForegroundColor Cyan

if ($Clean) {
    & $pio run --project-dir $projectDir --environment $envName --target clean
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

& $pio run --project-dir $projectDir --environment $envName
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build FAILED (exit $LASTEXITCODE)" -ForegroundColor Red
    exit $LASTEXITCODE
}

$outDir = Join-Path $projectDir ".pio\build\$envName"
$elrs = Join-Path $outDir 'firmware.elrs'
$bin  = Join-Path $outDir 'firmware.bin'

Write-Host ""
Write-Host "Build SUCCESS" -ForegroundColor Green
Write-Host "Output: $outDir"
foreach ($f in @($elrs, $bin)) {
    if (Test-Path $f) {
        $kb = [math]::Round((Get-Item $f).Length / 1KB, 1)
        Write-Host ("  {0,-15} {1,6} KB" -f (Split-Path $f -Leaf), $kb)
    }
}
Write-Host ""
Write-Host "To flash $Target via Flysky bootloader:" -ForegroundColor Yellow
Write-Host "  1. Copy firmware.elrs to SD card root (rename if your device expects ELRS.bin)"
Write-Host "  2. On the radio: SD Card menu -> flash ext. ELRS"
Write-Host "  See docs at https://github.com/richardclli/Flysky-ELRS/blob/main/docs/${Target}-flash.md"

if ($Open) {
    Start-Process explorer.exe $outDir
}
