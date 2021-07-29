(: Name: fn-id-14 :)
(: Description: Evaluation of fn:id for which the given IDREF contains a prefix. :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:id("p1:id5", $input-context1/ids:IDS[1]))