(: Name: Constr-compelem-constrmod-4 :)
(: Written by: Andreas Behm :)
(: Description: preserve decimal type :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(element elem {xs:decimal(($input-context//decimal[1]))}) cast as xs:integer