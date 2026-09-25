param(
    [Parameter(Mandatory=$true)][string]$ProjectDirectory,
    [ValidateSet('Debug','Release')][string]$Configuration='Release',
    [switch]$Clean
)
$ErrorActionPreference = 'Stop'
$suite = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$projects = (Resolve-Path -LiteralPath (Join-Path $suite 'Projects')).Path
$project = (Resolve-Path -LiteralPath $ProjectDirectory).Path
if (-not $project.StartsWith($projects + '\',
        [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'ProjectDirectory must be inside this suite Projects folder.'
}
$build = Join-Path $project 'out/build/windows-x64'
$binary = Join-Path $build $Configuration
$packageRoot = Join-Path $project 'out/package/windows-x64'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' +
    [Guid]::NewGuid().ToString('N').Substring(0,8)
$package = Join-Path $packageRoot ($Configuration + '-' + $stamp)
Push-Location $project
try {
    & cmake --preset windows-x64
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
    $arguments = @('--build','--preset',
        ('windows-x64-' + $Configuration.ToLowerInvariant()))
    if ($Clean) { $arguments += '--clean-first' }
    & cmake @arguments
    if ($LASTEXITCODE -ne 0) { throw 'Game build failed.' }
    $executables = @(Get-ChildItem -LiteralPath $binary -File |
        Where-Object { $_.Extension -eq '.exe' })
    if ($executables.Count -ne 1) {
        throw 'Expected one game EXE in the selected standalone configuration.'
    }
    $fontLicense = Join-Path $suite 'Engine/assets/fonts/Roboto/OFL.txt'
    if (-not (Test-Path -LiteralPath $fontLicense -PathType Leaf)) {
        throw 'Engine font license is missing.'
    }
    New-Item -ItemType Directory -Path $package -ErrorAction Stop | Out-Null
    Get-ChildItem -LiteralPath $binary -File |
        Where-Object { $_.Extension -in '.exe','.dll' } |
        ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $package }
   $assets = Join-Path $project 'Assets'
    if (-not (Test-Path -LiteralPath $assets -PathType Container)) {
        throw 'Project Assets directory is missing.'
    }
    Copy-Item -LiteralPath $assets -Destination (Join-Path $package 'Assets') -Recurse
    $licenses = Join-Path $project 'licenses'
    if (Test-Path -LiteralPath $licenses -PathType Container) {
        Copy-Item -LiteralPath $licenses -Destination (Join-Path $package 'licenses') -Recurse
    }
$fontDestination = Join-Path $package 'licenses/Roboto'
    New-Item -ItemType Directory -Path $fontDestination -Force | Out-Null
    Copy-Item -LiteralPath $fontLicense -Destination $fontDestination -Force
    foreach ($notice in 'THIRD_PARTY_NOTICES.md','LICENSE') {
        $file = Join-Path $suite $notice
        if (Test-Path -LiteralPath $file -PathType Leaf) {
            Copy-Item -LiteralPath $file -Destination $package
        }
    }
    Write-Host "Standalone package: $package"
    Write-Host 'Release users still need the matching Microsoft VC++ runtime.'
}
catch {
    Write-Warning "Packaging failed. Any incomplete package is left at: $package"
    throw
}
finally { Pop-Location }