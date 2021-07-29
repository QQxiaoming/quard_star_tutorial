(: Name: schema-import-18:)
(: Description: Evaluation of usage of xs:integer type as argument to fn:min(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:min(($input-context1/atomic:root/atomic:integer,$input-context1/atomic:root/atomic:integer))) eq 12678967543233
