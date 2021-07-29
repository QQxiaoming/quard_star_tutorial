(: Name: schema-import-31:)
(: Description: Evaluation of schema import with invalid namespace ("xml"). :)

(: insert-start :)
import schema namespace xml="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

"abc"