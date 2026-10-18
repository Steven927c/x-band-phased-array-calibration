param(
    [string]$BuildDirectory = "build-preflight"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$forbiddenExtensions = @(".AR2", ".log", ".bin", ".key", ".pem")
$forbiddenFiles = Get-ChildItem -Recurse -File | Where-Object {
    $_.FullName -notmatch "[\\/]build" -and
    $forbiddenExtensions -contains $_.Extension
}
if ($forbiddenFiles) {
    $forbiddenFiles | ForEach-Object { Write-Error "Forbidden publish file: $($_.FullName)" }
}

$textFiles = Get-ChildItem -Recurse -File | Where-Object {
    $_.FullName -notmatch "[\\/]build" -and
    $_.Name -notlike "preflight.*" -and
    $_.Extension -in @(".h", ".cpp", ".md", ".txt", ".yml", ".yaml", ".pro")
}
$patterns = @(
    "BEGIN (RSA|OPENSSH|EC) PRIVATE KEY",
    "AKIA[0-9A-Z]{16}",
    "192\.168\.",
    "g_Radar_Constant",
    "g_Servo.*Code",
    "Cmd\["
)
$matches = $textFiles | Select-String -Pattern $patterns
if ($matches) {
    $matches | Format-Table Path, LineNumber, Line -AutoSize
    throw "Potential private content found. Review before publishing."
}

cmake -S . -B $BuildDirectory -DRCT_BUILD_DEMO=ON -DRCT_BUILD_TESTS=ON
cmake --build $BuildDirectory --config Release
ctest --test-dir $BuildDirectory --output-on-failure -C Release

if (Get-Command git -ErrorAction SilentlyContinue) {
    git diff --check
    git status --short
}

Write-Host "Preflight completed. Manually finish OPEN_SOURCE_CHECKLIST.md before publishing."

