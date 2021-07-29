(: Name: fn-idref-16 :)
(: Description: Evaluation of fn:idref for which ID list have the same value for all its members but different case. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:idref(("id1","ID1"), $input-context1/ids:IDS[1])