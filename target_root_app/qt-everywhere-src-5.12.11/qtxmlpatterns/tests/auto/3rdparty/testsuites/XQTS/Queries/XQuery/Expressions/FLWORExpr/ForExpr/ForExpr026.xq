(: Name: ForExpr026 :)
(: Description: Missing 'in' :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file ($input-context//Folder)[1]/File
return $file/FileName