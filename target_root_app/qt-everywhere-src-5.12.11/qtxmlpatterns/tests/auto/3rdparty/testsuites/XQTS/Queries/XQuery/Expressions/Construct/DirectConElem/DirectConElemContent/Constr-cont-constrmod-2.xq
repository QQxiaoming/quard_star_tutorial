(: Name: Constr-cont-constrmod-2 :)
(: Written by: Andreas Behm :)
(: Description: preserve IDREFS type :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data(<elem>{$input-context//*:idrefs}</elem>/*))
