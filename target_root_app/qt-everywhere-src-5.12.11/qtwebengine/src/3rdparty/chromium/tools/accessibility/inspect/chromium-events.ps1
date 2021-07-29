# Powershell script to dump accessibility events for Chromium. Takes optional first argument with part of window title to disambiguate the desired process.
$all = ps | where {$_.ProcessName -eq 'chrome'} |where MainWindowTitle -like "*$($args[0])*chromium" | select id, MainWindowTitle
echo $all
echo ""
If (@($all).length -gt 1) {
  echo "Multiple matching processes, please disambuate: include part of the desired window's title as a first argument."
  exit
}
$id = ps | where {$_.ProcessName -eq 'chrome'} | where MainWindowTitle -like "*$($args[0])*chromium*" | select id -ExpandProperty id | Out-String
$id_arg = "--pid=" + $id
$exe = ".\ax_dump_events.exe"
& $exe $id_arg 
