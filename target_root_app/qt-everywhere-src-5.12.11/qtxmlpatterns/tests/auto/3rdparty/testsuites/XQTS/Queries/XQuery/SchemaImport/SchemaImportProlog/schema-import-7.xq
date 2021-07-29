(: Name: schema-import-7:)
(: Description: Evaluation of usage of xs:decimal type as argument to fn:avg(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:avg(($input-context1//atomic:decimal,$input-context1//atomic:decimal))) eq 12678967.543233