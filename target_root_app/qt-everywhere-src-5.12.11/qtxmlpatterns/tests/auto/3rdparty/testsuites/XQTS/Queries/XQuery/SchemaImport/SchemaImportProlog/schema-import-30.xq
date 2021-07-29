(: Name: schema-import-30:)
(: Description: Evaluation of schema import of numeric type (integer) and used on subtraction expression. :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

($input-context1//atomic:integer[1]) - ($input-context1//atomic:integer[1])