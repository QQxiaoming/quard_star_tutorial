(: Name: ForExpr021 :)
(: Description: Test order of iteration between two variables in the same 'for' statement :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $i in (1, 2), $j in (3, 4)
return ($i, $j)