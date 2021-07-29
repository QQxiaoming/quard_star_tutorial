(: Name: Orderexpr-7 :)
(: Written by: Andreas Behm :)
(: Description: fn:first ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {$input-context//part[@partid < 2]/fn:first()}
