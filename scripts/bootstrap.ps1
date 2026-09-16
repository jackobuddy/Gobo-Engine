$ErrorActionPreference = "Stop"
$suiteRoot = Split-Path -Parent $PSScriptRoot
git -C $suiteRoot submodule sync --recursive
git -C $suiteRoot submodule update --init --recursive
$vcpkgBootstrap = Join-Path $suiteRoot `
    "Engine/external/vcpkg/bootstrap-vcpkg.bat"
if (-not (Test-Path $vcpkgBootstrap)) {
    throw "vcpkg bootstrap script was not found."
}
& $vcpkgBootstrap -disableMetrics
if ($LASTEXITCODE -ne 0) { throw "vcpkg bootstrap failed." }
cmake --preset windows-x64 -S $suiteRoot
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }
Write-Host "Suite configured. Build with windows-x64-debug."