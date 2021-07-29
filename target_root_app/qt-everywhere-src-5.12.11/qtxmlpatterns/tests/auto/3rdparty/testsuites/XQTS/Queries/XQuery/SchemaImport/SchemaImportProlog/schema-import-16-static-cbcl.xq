(: Name: schema-import-16:)
(: Description: Evaluation of usage of xs:double type as argument to fn:max(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

fn:max(($input-context1/atomic:root/atomic:double,$input-context1/atomic:root/atomic:double))
