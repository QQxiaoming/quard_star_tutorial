(: Name: ForExprType012 :)
(: Description: Missing type declaration on type expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file as in (//Folder)[1]/File
return $file