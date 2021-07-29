(: Name: fn-id-21 :)
(: Description: Evaluation of fn:id for which $arg uses the fn:string-join function. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:string-join(("id","1"),""), $input-context1/ids:IDS[1])