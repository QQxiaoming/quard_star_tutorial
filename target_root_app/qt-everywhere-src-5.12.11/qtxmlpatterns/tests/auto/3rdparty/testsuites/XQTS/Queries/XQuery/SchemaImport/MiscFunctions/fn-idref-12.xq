(: Name: fn-idref-12 :)
(: Description: Evaluation of fn:idref used as ar argument to function fn:node-name() :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:node-name(fn:idref("id2", $input-context1/ids:IDS[1]))