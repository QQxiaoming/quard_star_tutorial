(: Name: ForExpr020 :)
(: Description: Multiple variable bindings followed by a trailing , :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder, 
	$file in $folder/File,
return $file/FileName
