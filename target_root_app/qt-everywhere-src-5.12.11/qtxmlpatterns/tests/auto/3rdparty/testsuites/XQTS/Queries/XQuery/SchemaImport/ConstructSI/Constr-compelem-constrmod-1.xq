(: Name: Constr-compelem-constrmod-1 :)
(: Written by: Andreas Behm :)
(: Description: strip IDREFS type :)

declare construction strip;

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data((element elem {$input-context//*:idrefs})/*))
