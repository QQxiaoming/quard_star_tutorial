(: Name: ST-WhereExpr002 :)
(: Description: Test 'where' clause with heterogenous sequences. First item is a value. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where (1, $file)
return $file/FileName