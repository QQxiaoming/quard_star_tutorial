(: Name: ForExpr023 :)
(: Description: Use positional variable in binding for new variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file at $pos in ($input-context//Folder)[1]/File, $pos2 in $pos+1
return (string($file/FileName[1]), $pos, $pos2)