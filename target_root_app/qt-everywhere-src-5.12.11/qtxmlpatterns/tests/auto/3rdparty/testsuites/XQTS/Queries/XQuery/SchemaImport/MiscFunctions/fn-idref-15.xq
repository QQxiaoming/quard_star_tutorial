(: Name: fn-idref-15 :)
(: Description: Evaluation of fn:idref for which ID list have the same value for all its members. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:idref(("id1","id1"), $input-context1/ids:IDS[1])