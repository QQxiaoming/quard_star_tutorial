(: Name: fn-id-19 :)
(: Description: Evaluation of fn:id for which $arg uses the fn:concat function. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:concat("i","d1"), $input-context1/ids:IDS[1])