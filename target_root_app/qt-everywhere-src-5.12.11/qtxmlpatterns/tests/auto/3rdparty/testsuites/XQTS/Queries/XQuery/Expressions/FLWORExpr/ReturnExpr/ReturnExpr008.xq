(: Name: ReturnExpr008 :)
(: Description: FLWOR expression returns constant value, independent of input bindings :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return 1