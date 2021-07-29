(: Name: Orderexpr-9 :)
(: Written by: Andreas Behm :)
(: Description: union ordered :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

ordered {$input-context//part[@partof = 1] union $input-context//part[@partid = 1] }
