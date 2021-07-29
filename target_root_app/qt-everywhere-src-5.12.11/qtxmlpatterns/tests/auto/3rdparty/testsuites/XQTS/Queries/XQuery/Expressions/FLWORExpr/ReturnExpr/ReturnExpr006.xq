(: Name: ReturnExpr006 :)
(: Description: FLWOR expression returns selected values :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return data( $file/FileName )