(: Name: fn-data-1:)
(: Description: Evaluation of the fn:data whose argument is a complex type with element only :)
(: content.  :)

(: insert-start :)
import schema namespace examples = "http://www.w3.org/XQueryTest/someExamples";
declare variable $input-context1 external;
(: insert-end :)

fn:data($input-context1/examples:E6-Root/examples:E6)