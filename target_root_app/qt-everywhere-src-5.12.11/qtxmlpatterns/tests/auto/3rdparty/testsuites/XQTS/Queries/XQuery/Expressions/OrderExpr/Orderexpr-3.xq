(: Name: Orderexpr-3 :)
(: Written by: Andreas Behm :)
(: Description: fn:last() ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {$input-context//part[@partid < 2]/fn:last()}
