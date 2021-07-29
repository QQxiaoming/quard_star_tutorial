(: Name: WhereExpr019 :)
(: Description: Typo on 'where' clause :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where_ true()
return $file/FileName