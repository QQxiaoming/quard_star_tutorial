(: Name: qname-cast-2 :)
(: Description: Evaluates casting a type derived from an xs:QName to an xs:QName type. :)


(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context external;
(: insert-end :)

let $var := myType:QNameBased("value1")
return $var cast as xs:QName