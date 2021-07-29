(: Name: Orderexpr-14 :)
(: Written by: Andreas Behm :)
(: Description: except unordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

unordered {$input-context//part[@partof < 2] except $input-context//part[@partid = 2] }
