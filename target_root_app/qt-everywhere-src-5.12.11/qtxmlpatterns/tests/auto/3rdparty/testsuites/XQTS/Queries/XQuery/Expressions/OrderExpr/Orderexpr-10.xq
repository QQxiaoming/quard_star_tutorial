(: Name: Orderexpr-10 :)
(: Written by: Andreas Behm :)
(: Description: union unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {$input-context//part[@partof = 1] union $input-context//part[@partid = 1] }
