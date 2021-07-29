(: Name: Constr-compelem-constrmod-8 :)
(: Written by: Andreas Behm :)
(: Description: preserve decimal type in attribute :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(element elem {xs:decimal($input-context//*:decimal[1]/@*:attr)}) cast as xs:integer

