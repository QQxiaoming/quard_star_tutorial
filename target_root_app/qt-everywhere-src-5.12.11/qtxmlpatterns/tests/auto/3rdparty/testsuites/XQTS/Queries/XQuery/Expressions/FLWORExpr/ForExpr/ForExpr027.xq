(: Name: ForExpr027 :)
(: Description: Wrong order for positional variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File at $pos
return (string($file/FileName[1]), $pos)