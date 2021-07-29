(: Name: ForExpr018 :)
(: Description: Multiple in statements for single variable binding :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in in $input-context//Folder
return $folder
