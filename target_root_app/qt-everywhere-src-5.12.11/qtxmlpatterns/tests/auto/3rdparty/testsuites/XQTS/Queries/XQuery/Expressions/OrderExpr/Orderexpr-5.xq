(: Name: Orderexpr-5 :)
(: Written by: Andreas Behm :)
(: Description: position predicate ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {$input-context//part[@partid < 2][2]}
