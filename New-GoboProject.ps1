param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z][A-Za-z0-9_]*$')]
    [string]$Name,
    [switch]$InitializeGit
)
$ErrorActionPreference = "Stop"
$suiteRoot = $PSScriptRoot
$template = Join-Path $suiteRoot "Projects/GoboGameTemplate"
$destination = Join-Path $suiteRoot "Projects/$Name"
if (-not (Test-Path $template)) {
    throw "GoboGameTemplate was not found: $template"
}
if (Test-Path $destination) {
    throw "Project already exists: $destination"
}
Copy-Item -LiteralPath $template -Destination $destination -Recurse
$copiedOut = Join-Path $destination "out"

if (Test-Path $copiedOut) {
    Remove-Item -LiteralPath $copiedOut -Recurse -Force
}
$cmakeFile = Join-Path $destination "CMakeLists.txt"
$cmake = Get-Content -LiteralPath $cmakeFile -Raw
$cmake = $cmake.Replace("GoboGameTemplate", $Name)
Set-Content -LiteralPath $cmakeFile -Value $cmake
$manifestFile = Join-Path $destination "vcpkg.json"
$manifest = Get-Content -LiteralPath $manifestFile -Raw
$manifest = $manifest.Replace("gobo-game-template", $Name.ToLower())
Set-Content -LiteralPath $manifestFile -Value $manifest
if ($InitializeGit) {
    git -C $destination init
}
Write-Host "Created project: $destination"
Write-Host "Open this folder in Visual Studio to begin."
pause