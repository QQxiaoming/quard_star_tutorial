(: Name: Constr-cont-enclexpr-4 :)
(: Written by: Andreas Behm :)
(: Description: enclosed expression with mix of atomic values and nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem>{1,$input-context//a,2,3,$input-context//comment(),4,5,$input-context//processing-instruction(),6,7,$input-context//text(),8}</elem>
