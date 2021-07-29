(: Name: Constr-compelem-constrmod-2 :)
(: Written by: Andreas Behm :)
(: Description: preserve IDREFS type :)

declare construction preserve;

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data((element elem {$input-context//*:idrefs})/*))
