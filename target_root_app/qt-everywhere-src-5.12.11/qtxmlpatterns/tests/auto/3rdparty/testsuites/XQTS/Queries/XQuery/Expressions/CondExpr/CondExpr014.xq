(: FileName: CondExpr014 :)
(: Purpose: Two if expressions at same depth in larger expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<out>{ ( if (3 != 2) then 16 else 0 )
     + ( if (8 = 7) then 4 else 1 ) }</out>