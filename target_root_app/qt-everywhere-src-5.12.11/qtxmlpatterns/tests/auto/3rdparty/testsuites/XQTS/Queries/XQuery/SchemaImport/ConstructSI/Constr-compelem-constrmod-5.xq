(: Name: Constr-compelem-constrmod-5 :)
(: Written by: Andreas Behm :)
(: Description: strip IDREFS type in attribute :)

declare construction strip;

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context external;
(: insert-end :)

fn:count(fn:data((element elem {$input-context//*:idrefs/@*:attr})/@*:attr))
