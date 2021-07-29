(: Name: fn-idref-13 :)
(: Description: Evaluation of fn:idref used as part of a node expression ("is" operand). :)
(: Compare same elements.  :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

exactly-one(fn:idref("id1", $input-context1/ids:IDS[1])) is exactly-one(fn:idref("id1", $input-context1/ids:IDS[1]))
