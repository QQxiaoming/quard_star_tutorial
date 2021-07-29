(: Name: WhereExpr019 :)
(: Description: Multiple 'where' clauses :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where true()
where false()
return $file/FileName