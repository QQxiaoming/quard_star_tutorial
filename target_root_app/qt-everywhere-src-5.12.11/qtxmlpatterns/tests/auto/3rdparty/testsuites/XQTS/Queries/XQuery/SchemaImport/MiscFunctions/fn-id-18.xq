(: Name: fn-id-18 :)
(: Description: Evaluation of fn:id for which IDREF uses the fn:upper-case function. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:upper-case("id5"), $input-context1/ids:IDS[1])