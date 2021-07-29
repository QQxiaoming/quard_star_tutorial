(: Name: ForExpr016 :)
(: Description: Interate over nodes in document and constant sequence. Return value based on both :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder, 
	$index in (1, 2, 3)
return $folder/File[$index]/FileName