(: Name: Constr-compelem-enclexpr-3 :)
(: Written by: Andreas Behm :)
(: Description: atomic values and nodes in computed element content :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {1,$input-context//a,2,3,$input-context//comment(),4,5,$input-context//processing-instruction(),6,7,$input-context//text(),8}
