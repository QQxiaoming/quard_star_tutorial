(: Name: fn-idref-23 :)
(: Description: Evaluation of fn:idref with declare ordering. :)

declare ordering ordered;

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:idref("id4", $input-context1/ids:IDS[1])