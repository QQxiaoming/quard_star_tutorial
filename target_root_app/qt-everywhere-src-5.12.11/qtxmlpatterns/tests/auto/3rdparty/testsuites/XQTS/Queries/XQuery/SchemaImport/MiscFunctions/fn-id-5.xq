(: Name: fn-id-5 :)
(: Description: Evaluation of fn:id with given IDREF matching a single element. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

 fn:id("id1", $input-context1/ids:IDS[1])