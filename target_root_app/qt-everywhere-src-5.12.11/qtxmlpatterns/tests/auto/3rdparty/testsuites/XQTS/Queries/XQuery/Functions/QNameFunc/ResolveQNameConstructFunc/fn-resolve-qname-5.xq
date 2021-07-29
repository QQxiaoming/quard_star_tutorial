(: Name: fn-resolve-qname-5 :)
(: Description: Evaluation of fn:resolve-qname function for which the give qname has a prefix and there is a namespace binding for the element. :)
(: uses fn:namespace-uri-from-QName to get namespace part.  Element is given as direct element:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:namespace-uri-from-QName(fn:resolve-QName("p1:name", <anElement xmlns:p1="http://example.com/examples">Some content</anElement>)))