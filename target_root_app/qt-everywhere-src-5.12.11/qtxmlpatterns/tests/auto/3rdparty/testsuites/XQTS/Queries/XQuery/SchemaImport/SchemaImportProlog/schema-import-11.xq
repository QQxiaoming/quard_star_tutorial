(: Name: schema-import-11:)
(: Description: Evaluation of usage of xs:decimal type as argument to fn:abs(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:abs($input-context1//atomic:decimal[1])) eq 12678967.543233