(: Name: CondExpr003 :)
(: Description: Simple if expression where boolean is a constant false :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if (fn:false()) then
    <elem1/>
else
    <elem2/>