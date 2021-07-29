(: Name: fn-id-11 :)
(: Description: Evaluation of fn:id with IDREF set to empty string. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:id("", $input-context1/ids:IDS[1]))