(: Name: fn-id-23 :)
(: Description: Evaluation of fn:id together with declare ordering. :)

declare ordering ordered;

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id("id1 id2", $input-context1/ids:IDS[1])