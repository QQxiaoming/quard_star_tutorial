(: Name: Constr-compelem-attr-1 :)
(: Written by: Andreas Behm :)
(: Description: attribute node following atomic value :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {1, $input-context//west/@mark}
