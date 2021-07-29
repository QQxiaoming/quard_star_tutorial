(: Name: ForExpr015 :)
(: Description: Return expression contains nested for expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder
return 
	for $file in $folder/File
	return string( $file/FileName[1] )