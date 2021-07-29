(: Name: Orderexpr-11 :)
(: Written by: Andreas Behm :)
(: Description: intersect ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {$input-context//part[@partof < 2] intersect $input-context//part[@partid = 1 or @partid > 2] }
