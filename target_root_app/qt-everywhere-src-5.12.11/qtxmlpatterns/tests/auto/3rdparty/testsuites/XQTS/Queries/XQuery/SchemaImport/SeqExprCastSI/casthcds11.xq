(: Name: casthcds11 :)
(: Description: Simple test for casting a float as a boolean:)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)


($input-context1/atomic:root/atomic:float) cast as xs:boolean
