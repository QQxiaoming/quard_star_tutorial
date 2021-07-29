(: Name: schema-import-13:)
(: Description: Evaluation of usage of xs:float type as argument to fn:max(). :)

(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

(fn:max(($input-context1//atomic:float,$input-context1//atomic:float))) eq xs:float(1.26743233E15)