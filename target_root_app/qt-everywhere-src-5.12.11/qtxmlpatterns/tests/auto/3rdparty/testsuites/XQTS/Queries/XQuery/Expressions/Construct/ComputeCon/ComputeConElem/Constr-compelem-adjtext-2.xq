(: Name: Constr-compelem-adjtext-2 :)
(: Written by: Andreas Behm :)
(: Description: merge adjacent atomic values and text nodes to text node :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

count((element elem {1, $input-context//text(), 'string'})/text())
