(: Name: fn-id-20 :)
(: Description: Evaluation of fn:id for which $arg uses the xs:string function. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id(xs:string("id1"), $input-context1/ids:IDS[1])