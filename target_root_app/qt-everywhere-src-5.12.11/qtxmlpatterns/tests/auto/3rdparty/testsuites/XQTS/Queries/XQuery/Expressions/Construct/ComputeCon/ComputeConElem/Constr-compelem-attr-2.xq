(: Name: Constr-compelem-attr-2 :)
(: Written by: Andreas Behm :)
(: Description: attribute node following node :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {element a {}, $input-context//west/@mark}
