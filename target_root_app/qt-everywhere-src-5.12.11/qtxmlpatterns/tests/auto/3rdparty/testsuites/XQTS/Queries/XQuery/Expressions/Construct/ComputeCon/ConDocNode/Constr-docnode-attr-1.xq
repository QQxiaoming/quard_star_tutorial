(: Name: Constr-docnode-attr-1 :)
(: Written by: Andreas Behm :)
(: Description: attribute in document constructor :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

document {$input-context//@mark}
