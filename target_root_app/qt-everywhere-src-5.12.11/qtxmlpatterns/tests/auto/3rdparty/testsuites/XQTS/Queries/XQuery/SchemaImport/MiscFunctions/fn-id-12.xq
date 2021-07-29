(: Name: fn-id-12 :)
(: Description: Evaluation of fn:id function that give first argument as argument to fn:substring. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:substring("1id3",2), $input-context1/ids:IDS[1])