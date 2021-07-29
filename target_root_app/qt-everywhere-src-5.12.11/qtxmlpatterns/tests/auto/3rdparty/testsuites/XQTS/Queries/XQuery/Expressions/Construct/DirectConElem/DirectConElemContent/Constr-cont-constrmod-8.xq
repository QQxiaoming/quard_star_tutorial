(: Name: Constr-cont-constrmod-8 :)
(: Written by: Andreas Behm :)
(: Description: preserve decimal type in attribute :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(<elem>{$input-context//*:decimal/@*:attr}</elem>)/@*:attr cast as xs:integer

