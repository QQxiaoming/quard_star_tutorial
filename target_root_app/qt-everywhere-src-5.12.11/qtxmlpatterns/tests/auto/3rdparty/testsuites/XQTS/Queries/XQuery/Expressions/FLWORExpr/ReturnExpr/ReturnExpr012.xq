(: Name: ReturnExpr012 :)
(: Description: Multiple return statements :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return $file 
return $file