(: Name: Constr-cont-constrmod-6 :)
(: Written by: Andreas Behm :)
(: Description: preserve IDREFS type in attribute :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data((<elem>{$input-context//*:idrefs/@*:attr}</elem>)/@*:attr))
