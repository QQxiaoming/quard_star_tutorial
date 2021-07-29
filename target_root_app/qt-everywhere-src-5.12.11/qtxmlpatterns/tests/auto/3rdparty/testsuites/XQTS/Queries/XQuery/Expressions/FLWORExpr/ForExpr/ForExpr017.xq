(: Name: ForExpr017 :)
(: Description: Multiple return statements from single for expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $input-context//Folder
return $folder
return $folder