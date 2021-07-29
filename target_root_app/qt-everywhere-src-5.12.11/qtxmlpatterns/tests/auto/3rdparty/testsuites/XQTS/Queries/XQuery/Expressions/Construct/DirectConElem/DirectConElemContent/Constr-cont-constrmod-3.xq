(: Name: Constr-cont-constrmod-3 :)
(: Written by: Andreas Behm :)
(: Description: strip decimal type :)

declare construction strip;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem>{$input-context//*:decimal}</elem>/* cast as xs:integer
