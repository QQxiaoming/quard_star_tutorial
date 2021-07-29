(: Name: fn-resolve-qname-11 :)
(: Description: Evaluation of fn:resolve-qname function for which the given qname has a prefix used together with xs:string and there is a namespace binding with prefix for the element. :)
(: uses fn:local-name-from-qname to get local part.  Element is given as direct element:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:local-name-from-QName(fn:resolve-QName(xs:string("p1:anElement"), <anElement xmlns:p1="http://example.com/examples">Some content</anElement>)))