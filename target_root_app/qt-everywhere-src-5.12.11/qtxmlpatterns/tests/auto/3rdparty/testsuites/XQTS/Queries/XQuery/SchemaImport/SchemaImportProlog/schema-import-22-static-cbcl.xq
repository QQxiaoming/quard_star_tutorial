(: Name: schema-import-22:)
(: Description: Evaluation of usage of xs:dateTime type as argument to fn:minutes-from-dateTime(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

fn:minutes-from-dateTime($input-context1/atomic:root/atomic:dateTime)
