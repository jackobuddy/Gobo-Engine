param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z][A-Za-z0-9_]*$')]
    [string]$Name,

    [switch]$InitializeGit
)

$ErrorActionPreference = 'Stop'

$suite = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$projects = (Resolve-Path -LiteralPath (Join-Path $suite 'Projects')).Path
$template = Join-Path $projects 'GoboGameTemplate'
$destination = [IO.Path]::GetFullPath((Join-Path $projects $Name))

if (-not $destination.StartsWith(
        $projects + '\',
        [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Destination must be inside Projects.'
}

if ($Name -match '^(con|prn|aux|nul|com[0-9]|lpt[0-9])$') {
    throw 'That name is reserved by Windows.'
}

if (-not (Test-Path -LiteralPath $template -PathType Container)) {
    throw "Template not found: $template"
}

if (Test-Path -LiteralPath $destination) {
    throw "Project already exists: $destination"
}

$excluded = @(
    'out',
    'build',
    'x64',
    '.vs',
    '.git',
    'vcpkg_installed',
    'CMakeUserPresets.json',
    '.codex',
    '.agents'
)

function Copy-TemplateTree(
    [string]$from,
    [string]$to
) {
    foreach ($item in Get-ChildItem -LiteralPath $from -Force) {
        if ($excluded -contains $item.Name) {
            continue
        }

        if ($item.Name -like '*.vcxproj.user') {
            continue
        }

        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
            throw "Template links require explicit handling: $($item.FullName)"
        }

        $target = Join-Path $to $item.Name

        if ($item.PSIsContainer) {
            New-Item -ItemType Directory -Path $target | Out-Null
            Copy-TemplateTree $item.FullName $target
        }
        else {
            Copy-Item -LiteralPath $item.FullName -Destination $target
        }
    }
}

New-Item -ItemType Directory -Path $destination | Out-Null

try {
    Copy-TemplateTree $template $destination

    $utf8 = New-Object System.Text.UTF8Encoding($false)

    $cmakeFile = Join-Path $destination 'CMakeLists.txt'
    $cmake = [IO.File]::ReadAllText($cmakeFile)
    $cmake = $cmake.Replace('GoboGameTemplate', $Name)
    [IO.File]::WriteAllText($cmakeFile, $cmake, $utf8)

    $manifestFile = Join-Path $destination 'vcpkg.json'
    $manifest = [IO.File]::ReadAllText($manifestFile) | ConvertFrom-Json
    $manifest.name = 'game-' + $Name.ToLowerInvariant().Replace('_', '-')

    [IO.File]::WriteAllText(
        $manifestFile,
        ($manifest | ConvertTo-Json -Depth 30),
        $utf8
    )

    if ($InitializeGit) {
        & git -C $destination init

        if ($LASTEXITCODE -ne 0) {
            throw 'Git initialization failed.'
        }
    }

    Push-Location $destination

    try {
        & cmake --fresh --preset windows-x64

        if ($LASTEXITCODE -ne 0) {
            throw 'Initial CMake configuration failed.'
        }
    }
    finally {
        Pop-Location
    }

    Write-Host "Created and configured project: $destination"
    Write-Host 'Open that folder in Visual Studio, then build and run the game.'
}
catch {
    Write-Warning "Project was created but setup did not complete: $destination"
    throw
}