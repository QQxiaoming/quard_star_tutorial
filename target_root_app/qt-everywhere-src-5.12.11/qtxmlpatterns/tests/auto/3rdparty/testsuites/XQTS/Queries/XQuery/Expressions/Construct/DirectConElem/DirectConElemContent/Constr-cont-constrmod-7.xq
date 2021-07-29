(: Name: Constr-cont-constrmod-7 :)
(: Written by: Andreas Behm :)
(: Description: strip decimal type in attribute :)

declare construction strip;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(<elem>{$input-context//*:decimal/@*:attr}</elem>)/@*:attr cast as xs:integer
