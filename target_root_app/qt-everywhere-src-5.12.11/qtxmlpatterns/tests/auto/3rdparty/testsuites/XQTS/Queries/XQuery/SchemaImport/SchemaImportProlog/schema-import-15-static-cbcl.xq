(: Name: schema-import-15:)
(: Description: Evaluation of usage of xs:decimal type as argument to fn:max(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:max(($input-context1/atomic:root/atomic:decimal,$input-context1/atomic:root/atomic:decimal))) eq 12678967.543233
