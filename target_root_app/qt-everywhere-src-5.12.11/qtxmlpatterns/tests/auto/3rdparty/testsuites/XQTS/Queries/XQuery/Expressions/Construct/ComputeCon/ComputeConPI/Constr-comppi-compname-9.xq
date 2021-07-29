(: Name: Constr-comppi-compname-9 :)
(: Written by: Andreas Behm :)
(: Description: QName as name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

processing-instruction {$input-context//*:QName} {'text'}
