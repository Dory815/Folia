[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'

function Find-RepositoryRoot([string]$StartPath) {
    $directory = Get-Item -LiteralPath $StartPath
    while ($null -ne $directory) {
        $buildScript = Join-Path $directory.FullName 'build_app.ps1'
        $project = Join-Path $directory.FullName 'src\app-winui\Folia.vcxproj'
        if ((Test-Path -LiteralPath $buildScript -PathType Leaf) -and
            (Test-Path -LiteralPath $project -PathType Leaf)) {
            return $directory.FullName
        }
        $directory = $directory.Parent
    }
    throw "Could not locate repository root above $StartPath"
}

function Quote-CmdArgument([string]$Value) {
    return '"' + $Value.Replace('"', '""') + '"'
}

function Require-File([string]$Path, [string]$Description) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing $Description`: $Path"
    }
}

$root = Find-RepositoryRoot $PSScriptRoot
$buildApp = Join-Path $root 'build_app.ps1'
$probeSource = Join-Path $PSScriptRoot 'winui-math-worker-probe.cpp'
$runtimeSource = Join-Path $root 'tools\test-support\windows_console_runtime.cpp'
$appSource = Join-Path $root 'src\app-winui'
$platformSource = Join-Path $root 'src\platform'
$configurationDirectory = Join-Path $root "build\app-winui\obj\x64\$Configuration"
$applicationDirectory = Join-Path $root "build\app-winui\bin\x64\$Configuration"
$scratchDirectory = Join-Path $root "build\math-port\worker-probe\$Configuration"
$nativeBuildDirectory = if ($Configuration -eq 'Debug') {
    Join-Path $root 'build\core'
} else {
    Join-Path $root 'build\core-release'
}
$nativeImportLibrary = Join-Path $nativeBuildDirectory 'FoliaNativeMath.lib'
$nativeRuntime = Join-Path $nativeBuildDirectory 'FoliaNativeMath.dll'
$probeExecutable = Join-Path $applicationDirectory 'FoliaMathWorkerProbe.exe'

New-Item -ItemType Directory -Force -Path $scratchDirectory | Out-Null
Write-Host "Building FoliaNativeMath and the WinUI application ($Configuration)..."
& $buildApp -Configuration $Configuration
if (-not $?) {
    throw "build_app.ps1 failed for $Configuration"
}

Require-File $probeSource 'worker probe source'
Require-File $runtimeSource 'console test runtime source'
Require-File (Join-Path $configurationDirectory 'pch.obj') 'WinUI PCH object'
Require-File (Join-Path $configurationDirectory 'MathJaxRenderer.obj') 'MathJaxRenderer object'
Require-File (Join-Path $configurationDirectory 'MathJaxSvgParser.obj') 'MathJaxSvgParser object'
Require-File (Join-Path $configurationDirectory 'editor_priority_work_queue.ixx.obj') 'priority queue module object'
Require-File (Join-Path $configurationDirectory 'Microsoft\STL\std.ixx.obj') 'MSVC std module object'
Require-File $nativeImportLibrary 'native math import library'
Require-File $nativeRuntime 'native math runtime DLL'

$vsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
Require-File $vsWhere 'Visual Studio locator'
$installation = & $vsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if ($LASTEXITCODE -ne 0 -or -not $installation) {
    throw 'Visual Studio C++ build tools were not found.'
}
$vcvars64 = Join-Path ($installation | Select-Object -First 1) 'VC\Auxiliary\Build\vcvars64.bat'
Require-File $vcvars64 'vcvars64.bat'

$runtimeObject = Join-Path $scratchDirectory 'windows_console_runtime.obj'
$probeObject = Join-Path $scratchDirectory 'winui_math_worker_probe.obj'
$compilerPdb = Join-Path $scratchDirectory 'worker-probe-compiler.pdb'
$linkerPdb = Join-Path $scratchDirectory 'FoliaMathWorkerProbe.pdb'
$stdoutPath = Join-Path $scratchDirectory 'probe.stdout.log'
$stderrPath = Join-Path $scratchDirectory 'probe.stderr.log'
$runtimeFlag = if ($Configuration -eq 'Debug') { '/MDd' } else { '/MD' }
$debugFlag = if ($Configuration -eq 'Debug') { '/Zi' } else { '' }
$compileFlags = @(
    '/nologo', '/std:c++latest', '/EHsc', $runtimeFlag, $debugFlag, '/utf-8',
    '/DWIN32_LEAN_AND_MEAN', ('/I' + (Quote-CmdArgument $appSource)),
    ('/I' + (Quote-CmdArgument $platformSource))
) | Where-Object { $_ }

$compileProbe = 'cl ' + (($compileFlags + @(
    '/c', (Quote-CmdArgument $probeSource),
    ('/Fo' + (Quote-CmdArgument $probeObject)),
    ('/Fd' + (Quote-CmdArgument $compilerPdb))
)) -join ' ')
$compileRuntime = 'cl ' + (($compileFlags + @(
    '/c', (Quote-CmdArgument $runtimeSource),
    ('/Fo' + (Quote-CmdArgument $runtimeObject)),
    ('/Fd' + (Quote-CmdArgument $compilerPdb))
)) -join ' ')
$linkInputs = @(
    (Quote-CmdArgument $probeObject),
    (Quote-CmdArgument $runtimeObject),
    (Quote-CmdArgument (Join-Path $configurationDirectory 'pch.obj')),
    (Quote-CmdArgument (Join-Path $configurationDirectory 'MathJaxRenderer.obj')),
    (Quote-CmdArgument (Join-Path $configurationDirectory 'MathJaxSvgParser.obj')),
    (Quote-CmdArgument (Join-Path $configurationDirectory 'editor_priority_work_queue.ixx.obj')),
    (Quote-CmdArgument (Join-Path $configurationDirectory 'Microsoft\STL\std.ixx.obj')),
    (Quote-CmdArgument $nativeImportLibrary)
)
$linkCommand = 'link /NOLOGO /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MANIFEST:NO /STACK:33554432 /OUT:' +
    (Quote-CmdArgument $probeExecutable) + ' /PDB:' + (Quote-CmdArgument $linkerPdb)
if ($Configuration -eq 'Debug') {
    $linkCommand += ' /DEBUG'
}
$linkCommand += ' ' + ($linkInputs -join ' ') + ' windowsapp.lib runtimeobject.lib'

$cmdLine = 'call ' + (Quote-CmdArgument $vcvars64) + ' >nul && ' +
    $compileProbe + ' && ' + $compileRuntime + ' && ' + $linkCommand
Write-Host 'Compiling and linking the direct MathJaxRenderer worker probe...'
& $env:ComSpec /d /s /c $cmdLine
if ($LASTEXITCODE -ne 0) {
    throw "Worker probe compilation/link failed with exit code $LASTEXITCODE"
}

Require-File $probeExecutable 'worker probe executable'
Require-File (Join-Path $applicationDirectory 'FoliaNativeMath.dll') 'deployed native math DLL'
$started = Get-Date
$process = Start-Process -FilePath $probeExecutable -WorkingDirectory $applicationDirectory `
    -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
# Retain the process handle before waiting, including on Windows PowerShell 5.1.
$processHandle = $process.Handle
if (-not $process.WaitForExit(60000)) {
    try { $process.Kill() } catch { }
    $process.WaitForExit()
    if (Test-Path -LiteralPath $stdoutPath) {
        Write-Host 'probe stdout:'
        Get-Content -LiteralPath $stdoutPath -Raw
    }
    if (Test-Path -LiteralPath $stderrPath) {
        Write-Host 'probe stderr:'
        Get-Content -LiteralPath $stderrPath -Raw
    }
    Write-Host 'probe exit code: timeout (probe process terminated)'
    throw 'Worker probe exceeded the 60-second deadline; only the probe process was terminated.'
}
$process.Refresh()
$exitCode = $process.ExitCode
$elapsed = (Get-Date) - $started
Write-Host 'probe stdout:'
if (Test-Path -LiteralPath $stdoutPath) { Get-Content -LiteralPath $stdoutPath -Raw }
Write-Host 'probe stderr:'
if (Test-Path -LiteralPath $stderrPath) { Get-Content -LiteralPath $stderrPath -Raw }
Write-Host "probe exit code: $exitCode; elapsed: $([math]::Round($elapsed.TotalSeconds, 2))s"
if ($exitCode -ne 0) {
    throw "Worker probe failed with exit code $exitCode"
}
