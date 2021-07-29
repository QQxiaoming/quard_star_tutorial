(: Name: Constr-compattr-doc-1 :)
(: Written by: Andreas Behm :)
(: Description: strip document nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {attribute attr {$input-context, $input-context}}
