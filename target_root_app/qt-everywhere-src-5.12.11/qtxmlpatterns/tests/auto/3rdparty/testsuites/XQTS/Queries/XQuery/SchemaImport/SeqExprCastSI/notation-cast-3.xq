(: Name: notation-cast-3 :)
(: Description: Evaluates casting a type derived from an xs:NOTATION to a type derived from xs:NOTATION. :)

(: insert-start :)
import schema namespace myType = "http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context external;
(: insert-end :)

myType:NOTATIONBased("myType:value1") cast as myType:NOTATIONBased