(: Name: fn-idref-17 :)
(: Description: Evaluation of fn:idref for which ID uses the fn lower case function. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:idref(fn:lower-case("ID1"), $input-context1/ids:IDS[1])