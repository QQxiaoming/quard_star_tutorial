(: Name: Constr-comppi-doc-1 :)
(: Written by: Andreas Behm :)
(: Description: strip document nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

processing-instruction pi {$input-context, $input-context}
