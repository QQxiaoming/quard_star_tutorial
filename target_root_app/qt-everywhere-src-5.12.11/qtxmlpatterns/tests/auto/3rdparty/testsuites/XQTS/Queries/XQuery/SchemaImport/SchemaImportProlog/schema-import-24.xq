(: Name: schema-import-24:)
(: Description: Evaluation of usage of xs:dateTime type as argument to fn:timezone-from-dateTime(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

fn:timezone-from-dateTime($input-context1//atomic:dateTime[1])