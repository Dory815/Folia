param(
    [Parameter(Mandatory = $true)]
    [string] $AppPath,
    [int] $CloseAfterMilliseconds = 750
)

$ErrorActionPreference = 'Stop'
$app = (Resolve-Path -LiteralPath $AppPath).Path
$fixture = Join-Path ([IO.Path]::GetTempPath()) ("folia-math-lifecycle-{0}.md" -f [guid]::NewGuid())
$formulas = 1..64 | ForEach-Object {
    $n = 100 + $_
    "`$`$\sum_{i=1}^{$n} \frac{i}{i^2+1}`$`$"
}
Set-Content -LiteralPath $fixture -Value ("# Math lifecycle smoke`n`n" + ($formulas -join "`n`n")) -Encoding utf8

if (-not ('FoliaLifecycleNativeMethods' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class FoliaLifecycleNativeMethods
{
    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool PostMessage(IntPtr hWnd, uint message, IntPtr wParam, IntPtr lParam);
}
'@
}

$process = $null
try {
    $argument = '"' + $fixture + '"'
    $process = Start-Process -FilePath $app -ArgumentList $argument -PassThru
    $watch = [Diagnostics.Stopwatch]::StartNew()
    while ($watch.Elapsed.TotalSeconds -lt 30) {
        $process.Refresh()
        if ($process.HasExited) { throw "Folia exited during startup with code $($process.ExitCode)." }
        if ($process.MainWindowHandle -ne [IntPtr]::Zero) { break }
        Start-Sleep -Milliseconds 100
    }
    if ($process.MainWindowHandle -eq [IntPtr]::Zero) { throw 'Folia did not expose its WinUI main window within 30 seconds.' }

    # Let the real document renderer enqueue native math work, then close the
    # actual WinUI window through WM_CLOSE so Closed/destruction runs normally.
    Start-Sleep -Milliseconds $CloseAfterMilliseconds
    if (-not [FoliaLifecycleNativeMethods]::PostMessage($process.MainWindowHandle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)) {
        throw "PostMessage(WM_CLOSE) failed: $([Runtime.InteropServices.Marshal]::GetLastWin32Error())"
    }
    if (-not $process.WaitForExit(15000)) {
        Stop-Process -Id $process.Id -Force
        throw 'Folia did not finish normal window shutdown within 15 seconds.'
    }
    if ($process.ExitCode -ne 0) { throw "Folia exited with code $($process.ExitCode)." }
    Write-Host 'WinUI MathJaxRenderer lifecycle smoke passed (normal window close, zero exit code).'
}
finally {
    if ($process) {
        $process.Refresh()
        if (-not $process.HasExited) { Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue }
    }
    Remove-Item -LiteralPath $fixture -Force -ErrorAction SilentlyContinue
}
