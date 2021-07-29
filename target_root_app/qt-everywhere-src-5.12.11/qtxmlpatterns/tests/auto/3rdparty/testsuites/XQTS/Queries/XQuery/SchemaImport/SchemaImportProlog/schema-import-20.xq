(: Name: schema-import-20:)
(: Description: Evaluation of usage of xs:double type as argument to fn:min(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

fn:min(($input-context1//atomic:double,$input-context1//atomic:double))