(: Name: Orderexpr-1 :)
(: Written by: Andreas Behm :)
(: Description: path expression ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {$input-context//part[@partid < 2]}
