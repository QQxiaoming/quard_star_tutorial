(: Name: fn-idref-9 :)
(: Description: Evaluation of fn:idref with multiple ID, but only one matching one element. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:idref(("id1", "nomatching"), $input-context1/ids:IDS[1])