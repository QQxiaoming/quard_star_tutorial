(: Name: WhereExpr017 :)
(: Description: Attempt to use multiple expressions in a 'where' clause :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where true(), true()
return $file/FileName