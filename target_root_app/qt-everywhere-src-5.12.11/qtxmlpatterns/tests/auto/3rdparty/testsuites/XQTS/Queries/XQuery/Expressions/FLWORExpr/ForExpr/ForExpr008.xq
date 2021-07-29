(: Name: ForExpr008 :)
(: Description: For+Where+Return - use special characters in variable name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $AaBbCc.-_Dd in $input-context/MyComputer/Drive1/Folder
where $AaBbCc.-_Dd/@creator = "Mani"
return $AaBbCc.-_Dd/FolderName
