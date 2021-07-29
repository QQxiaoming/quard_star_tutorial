(: Name: Constr-cont-constrmod-5 :)
(: Written by: Andreas Behm :)
(: Description: strip IDREFS type in attribute :)

declare construction strip;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data((<elem>{$input-context//*:idrefs/@*:attr}</elem>)/@*:attr))
