# Powershell script to dump entire accessibility tree for Firefox, all windows + tabs.
$hwnd = Get-Process | Where-Object {$_.ProcessName -eq 'firefox'} | where MainWindowTitle -like "*Firefox" | select MainWindowHandle -ExpandProperty MainWindowHandle | Out-String
$hwnd_arg = "--window=" + $hwnd
$exe = ".\ax_dump_tree.exe"
& $exe $hwnd_arg