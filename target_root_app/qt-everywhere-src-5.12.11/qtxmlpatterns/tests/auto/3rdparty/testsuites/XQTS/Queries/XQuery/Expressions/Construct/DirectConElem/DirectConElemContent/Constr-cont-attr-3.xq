(: Name: Constr-cont-attr-3 :)
(: Written by: Andreas Behm :)
(: Description: attribute node following empty sequence :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem>{()}{$input-context//west/@mark}</elem>
