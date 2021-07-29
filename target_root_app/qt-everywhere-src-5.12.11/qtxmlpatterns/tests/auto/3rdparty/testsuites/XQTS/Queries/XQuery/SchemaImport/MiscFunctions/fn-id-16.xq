(: Name: fn-id-16 :)
(: Description: Evaluation of fn:id for which IDREF list have the same value for all its members but different case. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id("id1 ID1", $input-context1/ids:IDS[1])