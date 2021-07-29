(: Name: fn-id-8 :)
(: Description: Evaluation of fn:id with multiple IDREF matching multiple elements. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id("id1 id2", $input-context1/ids:IDS[1])