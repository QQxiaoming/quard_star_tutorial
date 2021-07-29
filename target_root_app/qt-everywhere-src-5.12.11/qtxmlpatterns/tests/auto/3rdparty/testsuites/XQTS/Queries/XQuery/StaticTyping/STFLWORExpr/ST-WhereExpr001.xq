(: Name: ST-WhereExpr001 :)
(: Description: Test 'where' clause with heterogenous sequences. First item is a node :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
where ($file, 1)
return $file/FileName