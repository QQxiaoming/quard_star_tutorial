(: Name: fn-id-7 :)
(: Description: Evaluation of fn:id with given IDREF matching same element. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id("id2 id2", $input-context1/ids:IDS[1])