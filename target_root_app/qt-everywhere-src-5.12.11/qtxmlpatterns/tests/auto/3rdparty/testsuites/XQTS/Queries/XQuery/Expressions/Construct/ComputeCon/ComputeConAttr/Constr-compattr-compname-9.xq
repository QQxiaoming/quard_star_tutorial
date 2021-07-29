(: Name: Constr-compattr-compname-9 :)
(: Written by: Andreas Behm :)
(: Description: QName as name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

element elem {attribute {xs:QName('aQname')} {'text'}}
