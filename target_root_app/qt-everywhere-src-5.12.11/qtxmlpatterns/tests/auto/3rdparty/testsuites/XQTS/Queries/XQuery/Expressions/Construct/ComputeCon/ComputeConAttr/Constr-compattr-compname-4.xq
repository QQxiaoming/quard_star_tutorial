(: Name: Constr-compattr-compname-4 :)
(: Written by: Andreas Behm :)
(: Description: content of two nodes as name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {attribute {$input-context//a} {'text'}}
