(: Name: ReturnExpr014 :)
(: Description: FLWOR expression return statement depends on undefined variable :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return $undefined