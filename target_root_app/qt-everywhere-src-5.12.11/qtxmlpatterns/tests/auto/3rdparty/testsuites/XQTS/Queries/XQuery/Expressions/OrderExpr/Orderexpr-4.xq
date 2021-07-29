(: Name: Orderexpr-4 :)
(: Written by: Andreas Behm :)
(: Description: fn:last() unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {$input-context//part[@partid < 2]/fn:last()}
