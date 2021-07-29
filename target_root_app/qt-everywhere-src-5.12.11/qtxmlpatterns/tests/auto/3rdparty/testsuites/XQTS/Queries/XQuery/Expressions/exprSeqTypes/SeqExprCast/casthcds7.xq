(: Name: casthcds7 :)
(: Description: Simple test for evaluation a string casted as xs:string - XML Data Source :)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)


($input-context1/atomic:root/atomic:string) cast as xs:string
