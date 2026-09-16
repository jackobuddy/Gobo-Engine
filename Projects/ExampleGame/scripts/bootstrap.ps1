$ErrorActionPreference = "Stop"
$gameRoot = Split-Path -Parent $PSScriptRoot
$suiteRoot = Resolve-Path (Join-Path $gameRoot "../..")
$engineRoot = Join-Path $suiteRoot "Engine"
git -C $suiteRoot submodule update --init --recursive
$vcpkgBootstrap = Join-Path $engineRoot `
    "external/vcpkg/bootstrap-vcpkg.bat"
& $vcpkgBootstrap -disableMetrics
if ($LASTEXITCODE -ne 0) { throw "vcpkg bootstrap failed." }
cmake --preset windows-x64 -S $gameRoot
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }