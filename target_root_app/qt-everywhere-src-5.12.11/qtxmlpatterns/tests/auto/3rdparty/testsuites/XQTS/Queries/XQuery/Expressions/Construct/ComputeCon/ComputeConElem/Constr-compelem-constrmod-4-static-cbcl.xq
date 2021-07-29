(: Name: Constr-compelem-constrmod-4 :)
(: Written by: Andreas Behm :)
(: Description: preserve decimal type :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(element elem {xs:decimal(exactly-one($input-context//decimal))}) cast as xs:integer
