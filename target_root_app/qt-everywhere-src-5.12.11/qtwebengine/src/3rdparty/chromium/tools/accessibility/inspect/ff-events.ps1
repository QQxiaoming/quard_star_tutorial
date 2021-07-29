# Powershell script to dump entire accessibility tree for Firefox, all windows + tabs.
$id = ps | where Processname -like firefox | where MainWindowTitle -like "*Firefox" | select id -ExpandProperty id | Out-String
$id_arg = "--pid=" + $id
$exe = ".\ax_dump_events.exe"
& $exe $id_arg