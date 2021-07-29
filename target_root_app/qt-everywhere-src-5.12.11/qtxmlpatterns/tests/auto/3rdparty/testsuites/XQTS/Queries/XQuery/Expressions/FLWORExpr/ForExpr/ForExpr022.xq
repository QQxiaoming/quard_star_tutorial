(: Name: ForExpr022 :)
(: Description: FLWOR expressions with positional variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file at $pos in ($input-context//Folder)[1]/File
return (string($file/FileName[1]), $pos)