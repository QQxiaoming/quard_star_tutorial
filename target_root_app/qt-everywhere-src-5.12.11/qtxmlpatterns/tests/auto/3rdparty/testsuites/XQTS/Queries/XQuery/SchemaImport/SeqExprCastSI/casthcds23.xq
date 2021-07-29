
(: Name: casthcds23 :)
(: Description: Simple test for casting an integer as a float - XML Data Source :)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)


(($input-context1/atomic:root/atomic:integer) cast as xs:float) eq xs:float(12678967543233)
