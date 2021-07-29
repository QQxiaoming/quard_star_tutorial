(: Name: Orderexpr-13 :)
(: Written by: Andreas Behm :)
(: Description: except ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {$input-context//part[@partof < 2] except $input-context//part[@partid = 2] }
