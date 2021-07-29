(: Name: schema-import-23:)
(: Description: Evaluation of usage of xs:dateTime type as argument to fn:seconds-from-dateTime(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

fn:seconds-from-dateTime($input-context1//atomic:dateTime[1])