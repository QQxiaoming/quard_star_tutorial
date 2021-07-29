(: Name: schema-import-4:)
(: Description: Evaluation of usage of xs:boolean type as argument to fn:not(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

fn:not($input-context1//atomic:boolean)