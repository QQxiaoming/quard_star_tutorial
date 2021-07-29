(: Name: Orderexpr-8 :)
(: Written by: Andreas Behm :)
(: Description: fn:first unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {$input-context//part[@partid < 2]/fn:first()}
