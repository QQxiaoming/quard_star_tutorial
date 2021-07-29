(: Name: Orderexpr-12 :)
(: Written by: Andreas Behm :)
(: Description: intersect unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {$input-context//part[@partof < 2] intersect $input-context//part[@partid = 1 or @partid > 2] }
