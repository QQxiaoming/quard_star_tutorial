(: Name: Constr-compelem-constrmod-7 :)
(: Written by: Andreas Behm :)
(: Description: strip decimal type in attribute :)

declare construction strip;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(element elem {$input-context//*:decimal/@*:attr})/@*:attr cast as xs:integer
