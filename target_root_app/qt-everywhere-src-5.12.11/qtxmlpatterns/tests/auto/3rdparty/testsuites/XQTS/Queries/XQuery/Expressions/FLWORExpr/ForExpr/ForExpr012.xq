(: Name: ForExpr012 :)
(: Description: Nested FLWOR expressions :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $fileName in 
	for $file in $input-context//Folder/File
	return $file/FileName
return string( $fileName )