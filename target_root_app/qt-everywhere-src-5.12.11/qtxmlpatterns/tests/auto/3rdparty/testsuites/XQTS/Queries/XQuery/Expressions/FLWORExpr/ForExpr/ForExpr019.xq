(: Name: ForExpr019 :)
(: Description: Multiple for statements for single variable binding :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for for $folder in $input-context//Folder
return $folder
