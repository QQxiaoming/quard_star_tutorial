(: Name: fn-idref-11 :)
(: Description: Evaluation of fn:idref with ID set to empty string. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:idref("", $input-context1/ids:IDS[1]))