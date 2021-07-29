(: FileName: CondExpr012 :)
(: Description: Adapted from example in spec; test expression is a relational expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{ if ($input-context/doc/widget1/@unit-cost = $input-context/doc/widget2/@unit-cost)
     then $input-context/doc/widget1/@name else $input-context/doc/widget2/@name }</out>