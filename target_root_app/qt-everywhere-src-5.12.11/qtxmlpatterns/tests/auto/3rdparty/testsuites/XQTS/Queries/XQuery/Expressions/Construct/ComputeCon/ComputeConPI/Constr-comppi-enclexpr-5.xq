(: Name: Constr-comppi-enclexpr-5 :)
(: Written by: Andreas Behm :)
(: Description: enclosed expression in computed processing-instruction node - nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

processing-instruction pi {$input-context/root}
