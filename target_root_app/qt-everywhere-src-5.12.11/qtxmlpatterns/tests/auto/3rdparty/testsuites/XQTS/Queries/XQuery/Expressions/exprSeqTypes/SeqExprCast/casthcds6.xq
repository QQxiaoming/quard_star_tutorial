(: Name: casthcds6 :)
(: Description: Simple test for evaluation of a simple expression casted as xs:string - XML Data Source :)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)


($input-context1/atomic:root/atomic:integer) cast as xs:string
