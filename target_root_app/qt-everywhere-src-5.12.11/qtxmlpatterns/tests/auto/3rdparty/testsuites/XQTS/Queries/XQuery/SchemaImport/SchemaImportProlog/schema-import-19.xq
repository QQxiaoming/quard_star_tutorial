(: Name: schema-import-19:)
(: Description: Evaluation of usage of xs:decimal type as argument to fn:min(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:min(($input-context1//atomic:decimal,$input-context1//atomic:decimal))) eq 12678967.543233