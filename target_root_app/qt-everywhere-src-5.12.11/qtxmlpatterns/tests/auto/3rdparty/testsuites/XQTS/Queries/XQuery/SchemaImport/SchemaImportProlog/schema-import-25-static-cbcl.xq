(: Name: schema-import-25:)
(: Description: Evaluation of schema import of numeric type (float) and used on addition expression. :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(($input-context1/atomic:root/atomic:float) + ($input-context1/atomic:root/atomic:float)) eq xs:float(2.53486466E15)
