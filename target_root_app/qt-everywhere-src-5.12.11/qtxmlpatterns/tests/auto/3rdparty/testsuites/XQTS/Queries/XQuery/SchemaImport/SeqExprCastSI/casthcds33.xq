(: Name: casthcds33 :)
(: Description: Simple test for casting an date as a dateTime - XML Data Source :)
(: insert-start :)
import schema namespace atomic="http://www.w3.org/XQueryTest";
declare variable $input-context1 external;
(: insert-end :)


($input-context1/atomic:root/atomic:date) cast as xs:dateTime
