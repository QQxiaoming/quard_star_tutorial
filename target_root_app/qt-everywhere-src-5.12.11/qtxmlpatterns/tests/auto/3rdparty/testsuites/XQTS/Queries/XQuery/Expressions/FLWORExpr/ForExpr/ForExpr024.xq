(: Name: ForExpr024 :)
(: Description: Reuse existing variable name for positional variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file at $file in ($input-context//Folder)[1]/File
return (string($file/FileName[1]), $file)