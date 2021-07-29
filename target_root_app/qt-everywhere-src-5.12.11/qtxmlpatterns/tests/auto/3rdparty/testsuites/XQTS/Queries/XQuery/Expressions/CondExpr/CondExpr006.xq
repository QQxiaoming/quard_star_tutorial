(: FileName: CondExpr006 :)
(: Description: then-expression has another if expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{ if (1 != 0)
     then if (4 != 5) then 1 else 2
     else 3 }</out>