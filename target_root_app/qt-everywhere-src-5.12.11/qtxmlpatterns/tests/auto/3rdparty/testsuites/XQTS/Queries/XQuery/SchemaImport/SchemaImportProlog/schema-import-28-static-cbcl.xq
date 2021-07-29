(: Name: schema-import-28:)
(: Description: Evaluation of schema import of numeric type (double) and used on addition expression. :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

($input-context1/atomic:root/atomic:double) + ($input-context1/atomic:root/atomic:double)
