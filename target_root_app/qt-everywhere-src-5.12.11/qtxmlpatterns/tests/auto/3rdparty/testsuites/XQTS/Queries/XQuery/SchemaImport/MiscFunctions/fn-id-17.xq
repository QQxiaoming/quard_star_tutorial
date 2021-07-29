(: Name: fn-id-17 :)
(: Description: Evaluation of fn:id for which IDREF uses the fn lower case function. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:lower-case("ID1"), $input-context1/ids:IDS[1])