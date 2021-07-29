(: Name: casthcds10 :)
(: Description: Simple test for casting a float as a double:)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)

xs:float((($input-context1/atomic:root/atomic:float) cast as xs:double)) eq xs:float(1267.43233E12)