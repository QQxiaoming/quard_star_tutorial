(: Name: schema-import-14:)
(: Description: Evaluation of usage of xs:integer type as argument to fn:max(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:max(($input-context1//atomic:integer,$input-context1//atomic:integer))) eq 12678967543233