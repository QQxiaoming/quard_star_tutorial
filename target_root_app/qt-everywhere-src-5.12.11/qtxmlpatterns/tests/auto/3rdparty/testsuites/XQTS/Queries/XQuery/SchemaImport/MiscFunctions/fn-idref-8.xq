(: Name: fn-idref-8 :)
(: Description: Evaluation of fn:idref with multiple ID matching multiple elements. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:idref(("id1", "id2"), $input-context1/ids:IDS[1])