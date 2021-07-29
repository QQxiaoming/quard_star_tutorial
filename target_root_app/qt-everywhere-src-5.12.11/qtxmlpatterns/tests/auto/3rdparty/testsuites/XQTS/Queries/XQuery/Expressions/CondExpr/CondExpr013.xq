(: FileName: CondExpr013 :)
(: Purpose: test expression has another if expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{ if (if (5 != 3) then fn:true() else fn:empty($input-context/doc/widget1))
     then "search" else "assume" }</out>