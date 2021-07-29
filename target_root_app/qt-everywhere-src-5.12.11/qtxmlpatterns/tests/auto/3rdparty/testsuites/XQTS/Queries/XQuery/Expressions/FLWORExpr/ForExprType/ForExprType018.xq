(: Name: ForExprType018 :)
(: Description: Match a 'node' type :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file as node() in ($input-context//Folder)[1]/File
return $file/FileName
