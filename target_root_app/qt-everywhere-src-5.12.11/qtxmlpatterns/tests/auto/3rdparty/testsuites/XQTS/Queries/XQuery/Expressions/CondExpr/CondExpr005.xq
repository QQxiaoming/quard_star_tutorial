(: Name: CondExpr005 :)
(: Description: Empty node set from a path expression as test condition :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if ($input-context//NodeDoesNotExist) then
    <elem1/>
else
    <elem2/>