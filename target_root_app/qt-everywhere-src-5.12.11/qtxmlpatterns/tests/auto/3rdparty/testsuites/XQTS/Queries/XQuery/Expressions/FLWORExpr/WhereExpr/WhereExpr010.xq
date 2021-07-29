(: Name: WhereExpr010 :)
(: Description: Test 'where' expression returning a sequence with multiple values :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where data( $file/SecurityObject//right )
return $file/FileName