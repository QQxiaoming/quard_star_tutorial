(: Name: Constr-compattr-compname-8 :)
(: Written by: Andreas Behm :)
(: Description: NCName as name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {attribute {$input-context//*:NCName[1]} {'text'}}
