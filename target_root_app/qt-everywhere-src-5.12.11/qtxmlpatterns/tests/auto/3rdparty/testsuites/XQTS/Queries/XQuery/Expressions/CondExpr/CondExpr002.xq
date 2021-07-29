(: Name: CondExpr002 :)
(: Description: Simple if expression where boolean is a constant true :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

if (fn:true()) then
    <elem1/>
else
    <elem2/>