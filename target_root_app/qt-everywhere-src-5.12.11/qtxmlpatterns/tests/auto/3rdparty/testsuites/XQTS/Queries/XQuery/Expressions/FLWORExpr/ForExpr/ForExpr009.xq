(: Name: ForExpr009 :)
(: Description: For+Return - error, use variable in it's own declaration  :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $a in $a/*
return $a
