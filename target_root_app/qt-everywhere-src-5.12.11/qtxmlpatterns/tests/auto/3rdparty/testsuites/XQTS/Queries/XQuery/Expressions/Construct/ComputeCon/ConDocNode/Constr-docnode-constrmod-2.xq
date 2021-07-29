(: Name: Constr-docnode-constrmod-2 :)
(: Written by: Andreas Behm :)
(: Description: preserve IDREFS type :)

declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data((document {$input-context//*:idrefs})/*))
