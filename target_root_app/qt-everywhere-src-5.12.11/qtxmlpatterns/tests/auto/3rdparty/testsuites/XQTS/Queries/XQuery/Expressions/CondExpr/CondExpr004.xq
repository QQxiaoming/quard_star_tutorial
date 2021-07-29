(: Name: CondExpr004 :)
(: Description: Node set from a path expression as test condition :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if ($input-context//CompanyName) then
    <elem1/>
else
    <elem2/>