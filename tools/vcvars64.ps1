Param(
    [Parameter(Position = 0, HelpMessage = 'Visual Studio version.')]
    [ValidateNotNull()]
    [ValidatePattern('2019|2022')]
    [Int]$Version = 2019
)

Set-StrictMode -Version 3.0

$programFiles = $Version -gt 2019 ? 'Program Files' : 'Program Files (x86)'

cmd.exe /c "call `"C:\$programFiles\Microsoft Visual Studio\$Version\Community\VC\Auxiliary\Build\vcvars64.bat`" && set > %TEMP%\vcvars64_$Version.tmp"

Get-Content "$env:TEMP\vcvars64_$Version.tmp" | Foreach-Object {
    if ($_ -match "^(.*?)=(.*)$") {
        Set-Content "env:\$($matches[1])" $matches[2]
    }
}

