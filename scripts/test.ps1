param(
    [ValidateSet("Debug", "Release", "Both")]
    [string]$Configuration = "Both"
)

$ErrorActionPreference = "Stop"
$suiteRoot = Split-Path -Parent $PSScriptRoot

function Invoke-TestConfiguration([string]$name)
{
    $preset = "windows-x64-tests-$($name.ToLowerInvariant())"

    & cmake --build --preset $preset
    if ($LASTEXITCODE -ne 0)
    {
        throw "Test build failed for $name."
    }

    & ctest --preset $preset
    if ($LASTEXITCODE -ne 0)
    {
        throw "Tests failed for $name."
    }
}

Push-Location $suiteRoot

try
{
    & cmake --preset windows-x64
    if ($LASTEXITCODE -ne 0)
    {
        throw "CMake configuration failed."
    }

    if ($Configuration -eq "Debug" -or $Configuration -eq "Both")
    {
        Invoke-TestConfiguration "Debug"
    }

    if ($Configuration -eq "Release" -or $Configuration -eq "Both")
    {
        Invoke-TestConfiguration "Release"
    }

    Write-Host "Requested engine test configurations passed."
}
finally
{
    Pop-Location
}