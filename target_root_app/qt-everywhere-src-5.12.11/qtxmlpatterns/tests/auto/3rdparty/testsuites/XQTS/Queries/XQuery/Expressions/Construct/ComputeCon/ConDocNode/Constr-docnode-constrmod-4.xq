(: Name: Constr-docnode-constrmod-4 :)
(: Written by: Andreas Behm :)
(: Description: preserve decimal type :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(document {$input-context//*:decimal})/* cast as xs:integer

