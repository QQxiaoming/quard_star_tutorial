(: Name: ForExpr007 :)
(: Description: For+Return - use special character in variable name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $AaBbCc.-_Dd in $input-context/MyComputer/Drive1/Folder
return $AaBbCc.-_Dd/FolderName
