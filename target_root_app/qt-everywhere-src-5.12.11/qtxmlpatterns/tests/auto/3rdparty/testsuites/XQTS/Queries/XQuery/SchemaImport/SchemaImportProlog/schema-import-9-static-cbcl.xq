(: Name: schema-import-9:)
(: Description: Evaluation of usage of xs:float type as argument to fn:abs(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:abs($input-context1/atomic:root/atomic:float)) eq xs:float(1.26743233E15)
