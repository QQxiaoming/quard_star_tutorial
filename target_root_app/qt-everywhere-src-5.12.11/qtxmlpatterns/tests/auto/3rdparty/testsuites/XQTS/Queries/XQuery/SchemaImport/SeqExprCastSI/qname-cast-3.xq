(: Name: qname-cast-3 :)
(: Description: Evaluates casting a type derived from an xs:QName to a type derived from xs:QName. :)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context external;
(: insert-end :)

let $var := myType:QNameBased("value1")
return $var cast as myType:QNameBased