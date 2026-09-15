$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

Write-Host "Synchronizing repository dependencies..."
git -C $repoRoot submodule sync --recursive
git -C $repoRoot submodule update --init --recursive

$vcpkgBootstrap = Join-Path $repoRoot "external/vcpkg/bootstrap-vcpkg.bat"
if (-not (Test-Path $vcpkgBootstrap)) {
    throw "vcpkg bootstrap script was not found. Check the submodule."
}

Write-Host "Bootstrapping vcpkg..."
& $vcpkgBootstrap -disableMetrics
if ($LASTEXITCODE -ne 0) { throw "vcpkg bootstrap failed." }

Write-Host "Configuring Gobo Engine..."
cmake --preset windows-x64
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }

Write-Host "Setup complete. Build with the windows-x64-debug preset."