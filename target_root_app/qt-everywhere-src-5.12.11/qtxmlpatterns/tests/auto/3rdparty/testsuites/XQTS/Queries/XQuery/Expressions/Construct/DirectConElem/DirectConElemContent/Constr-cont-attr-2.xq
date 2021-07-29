(: Name: Constr-cont-attr-2 :)
(: Written by: Andreas Behm :)
(: Description: attribute node following node :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem><a/>{$input-context//west/@mark}</elem>
