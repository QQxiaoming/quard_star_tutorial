(: Name: schema-import-12:)
(: Description: Evaluation of usage of xs:double type as argument to fn:abs(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

fn:abs($input-context1//atomic:double[1])