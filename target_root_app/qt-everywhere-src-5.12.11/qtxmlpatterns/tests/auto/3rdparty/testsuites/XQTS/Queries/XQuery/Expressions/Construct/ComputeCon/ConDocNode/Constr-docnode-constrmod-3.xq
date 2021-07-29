(: Name: Constr-docnode-constrmod-3 :)
(: Written by: Andreas Behm :)
(: Description: strip decimal type :)

declare construction strip;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(document {$input-context//*:decimal})/* cast as xs:integer
