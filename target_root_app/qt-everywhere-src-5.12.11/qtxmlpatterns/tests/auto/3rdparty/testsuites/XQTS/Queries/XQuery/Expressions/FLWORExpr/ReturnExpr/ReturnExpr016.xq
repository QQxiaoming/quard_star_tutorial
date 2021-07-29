(: Name: ReturnExpr016 :)
(: Description: Return value of positional variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file at $offset in ($input-context//Folder)[1]/File
return
<File>{ $file/@name }{ attribute offset{ $offset }}</File>
