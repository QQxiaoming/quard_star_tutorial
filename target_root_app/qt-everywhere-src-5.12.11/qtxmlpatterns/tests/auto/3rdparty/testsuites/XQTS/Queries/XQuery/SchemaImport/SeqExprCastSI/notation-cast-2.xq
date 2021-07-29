(: Name: notation-cast-2 :)
(: Description: Evaluates casting a type derived from an xs:NOTATION to an xs:NOTATION type. :)


(: insert-start :)
import schema namespace myType = "http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context external;
(: insert-end :)

myType:NOTATIONBased("myType:value1") cast as xs:NOTATION