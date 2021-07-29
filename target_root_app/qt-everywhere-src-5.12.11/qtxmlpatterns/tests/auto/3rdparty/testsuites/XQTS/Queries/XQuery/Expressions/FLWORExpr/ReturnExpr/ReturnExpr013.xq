(: Name: ReturnExpr013 :)
(: Description: Missing 'return' statement in FLWOR expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File