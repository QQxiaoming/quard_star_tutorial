(: Name: casthcds14 :)
(: Description: Simple test for casting a double as a float - XML Data Source :)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)


(($input-context1/atomic:root/atomic:double) cast as xs:float) eq xs:float(1267.43233E12)
