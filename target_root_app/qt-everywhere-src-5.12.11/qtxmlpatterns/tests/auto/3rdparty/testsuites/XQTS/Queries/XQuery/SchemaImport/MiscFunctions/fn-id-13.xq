(: Name: fn-id-13 :)
(: Description: Evaluation of fn:id, where the same IDREF makes refence to the same element. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id("id4", $input-context1/ids:IDS[1])