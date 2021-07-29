(: Name: Constr-docnode-adjtext-2 :)
(: Written by: Andreas Behm :)
(: Description: merge adjacent atomic values and text nodes to text node :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

count((document {1, $input-context//text(), 'string'})/text())
