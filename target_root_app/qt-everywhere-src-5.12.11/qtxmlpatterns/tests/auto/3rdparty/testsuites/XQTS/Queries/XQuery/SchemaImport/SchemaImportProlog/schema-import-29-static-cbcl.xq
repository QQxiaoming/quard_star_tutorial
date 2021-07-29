(: Name: schema-import-29:)
(: Description: Evaluation of schema import of numeric type (integer) and used on division (idiv operator) expression. :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

($input-context1/atomic:root/atomic:integer) idiv ($input-context1/atomic:root/atomic:integer)
