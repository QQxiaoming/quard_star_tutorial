(: Name: ForExpr014 :)
(: Description: Incorrect syntax for nested loop. Multiple return statements at the same level :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $fileName in $input-context//File/FileName
	for $folderName in $input-context//Folder/FolderName
	return $folderName
return $fileName