(: Name: ReturnExpr009 :)
(: Description: FLWOR expression returns node from document, independent of input bindings :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return ($input-context//FileName)[1]