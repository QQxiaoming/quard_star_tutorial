(: Name: casthcds9 :)
(: Description: Simple test for casting a float as a string - XML Data Source :)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)


(($input-context1/atomic:root/atomic:float) cast as xs:string)
