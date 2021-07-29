(: Name: qname-cast-1 :)
(: Description: Evaluates casting an xs:QName type to another xs:QName type. :)

(: insert-start :)
import schema namespace myType = "http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context external;
(: insert-end :)

xs:QName("value1") cast as xs:QName