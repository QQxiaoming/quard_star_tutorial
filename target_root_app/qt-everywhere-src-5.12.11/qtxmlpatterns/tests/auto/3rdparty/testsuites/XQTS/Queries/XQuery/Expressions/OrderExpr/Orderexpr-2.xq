(: Name: Orderexpr-2 :)
(: Written by: Andreas Behm :)
(: Description: path expression unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {$input-context//part[@partid < 2]}
