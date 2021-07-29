(: Name: WhereExpr018 :)
(: Description: Use of undefined variable in 'where' clause :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where $undefined
return $file/FileName