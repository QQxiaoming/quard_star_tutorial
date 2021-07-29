(: Name: schema-import-10:)
(: Description: Evaluation of usage of xs:integer type as argument to fn:abs(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:abs($input-context1//atomic:integer[1])) eq 12678967543233