(: Name: ForExpr013 :)
(: Description: Multiple variables based off the same input context :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $fileName in $input-context//File/FileName, 
	$folderName in $input-context//Folder/FolderName
return ($folderName, $fileName)