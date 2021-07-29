(: Name: ReturnExpr005 :)
(: Description: FLWOR expression returns selected element nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return $file/FileName