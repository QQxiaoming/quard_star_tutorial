(: Name: Constr-cont-constrmod-1 :)
(: Written by: Andreas Behm :)
(: Description: strip IDREFS type :)

declare construction strip;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data(<elem>{$input-context//*:idrefs}</elem>/*))
