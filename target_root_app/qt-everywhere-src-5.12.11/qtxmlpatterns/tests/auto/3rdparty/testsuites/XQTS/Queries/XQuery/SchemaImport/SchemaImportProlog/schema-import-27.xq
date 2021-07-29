(: Name: schema-import-27:)
(: Description: Evaluation of schema import of numeric type (integer) and used on addition expression. :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(($input-context1//atomic:integer[1]) + ($input-context1//atomic:integer[1])) eq 2.5357935086466E13