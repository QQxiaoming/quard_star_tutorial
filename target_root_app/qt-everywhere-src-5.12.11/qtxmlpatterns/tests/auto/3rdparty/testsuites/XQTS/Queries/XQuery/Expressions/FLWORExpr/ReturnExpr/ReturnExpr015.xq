(: Name: ReturnExpr015 :)
(: Description: Variable bound to value from return statement :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in
	for $folder in ($input-context//Folder)[1]
	return $folder/File
return $file/FileName