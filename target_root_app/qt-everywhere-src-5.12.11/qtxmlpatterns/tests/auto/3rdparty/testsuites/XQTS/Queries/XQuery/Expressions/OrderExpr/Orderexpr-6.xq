(: Name: Orderexpr-6 :)
(: Written by: Andreas Behm :)
(: Description: position predicate unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {$input-context//part[@partid < 2][2]}
