(: Name: fn-idref-6 :)
(: Description: Evaluation of fn:idref with given ID not matching a single element. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:idref("nomatchingid", $input-context1/ids:IDS[1]))