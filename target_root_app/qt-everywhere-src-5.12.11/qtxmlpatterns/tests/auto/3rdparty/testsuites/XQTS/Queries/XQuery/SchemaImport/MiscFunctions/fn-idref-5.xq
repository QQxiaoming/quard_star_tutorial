(: Name: fn-idref-5 :)
(: Description: Evaluation of fn:idref with given ID matching a single element. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:idref("id1",$input-context1/ids:IDS[1])