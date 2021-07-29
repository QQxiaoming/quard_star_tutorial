(: Name: WhereExpr014 :)
(: Description: Test 'where' expression returning a sequence with a constant boolean value :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where true()
return $file/FileName