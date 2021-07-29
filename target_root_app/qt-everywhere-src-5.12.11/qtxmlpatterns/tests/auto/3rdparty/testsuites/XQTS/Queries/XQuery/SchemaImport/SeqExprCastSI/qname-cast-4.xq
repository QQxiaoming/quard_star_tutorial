(: Name: qname-cast-4 :)
(: Description: Evaluates casting a xs:QName type to a type derived from xs:QName. :)

(: insert-start :)
import schema namespace myType="http://www.w3.org/XQueryTest/userDefinedTypes";
declare variable $input-context external;
(: insert-end :)

let $var := xs:QName("value1")
return $var cast as myType:QNameBased