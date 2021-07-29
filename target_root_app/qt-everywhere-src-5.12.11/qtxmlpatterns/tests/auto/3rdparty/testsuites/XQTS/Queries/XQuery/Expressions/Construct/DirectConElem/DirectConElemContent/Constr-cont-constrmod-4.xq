(: Name: Constr-cont-constrmod-4 :)
(: Written by: Andreas Behm :)
(: Description: preserve decimal type :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<elem>{$input-context//*:decimal}</elem>/* cast as xs:integer

