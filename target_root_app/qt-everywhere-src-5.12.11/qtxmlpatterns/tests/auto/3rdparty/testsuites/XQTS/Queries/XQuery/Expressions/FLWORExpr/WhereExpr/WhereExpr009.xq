(: Name: WhereExpr009 :)
(: Description: Test 'where' expression returning a sequence with one or more nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where $file/SecurityObject/Denies/Deny/security/right
return $file/FileName