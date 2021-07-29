(: Name: Constr-docnode-attr-2 :)
(: Written by: Andreas Behm :)
(: Description: attribute in document constructor :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

document {<a/>, $input-context//@mark}
