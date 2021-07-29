(: Name: WhereExpr016 :)
(: Description: Test 'where' clause based on a positional variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file at $offset in ($input-context//Folder)[1]/File
where $offset mod 2 = 1
return $file/FileName