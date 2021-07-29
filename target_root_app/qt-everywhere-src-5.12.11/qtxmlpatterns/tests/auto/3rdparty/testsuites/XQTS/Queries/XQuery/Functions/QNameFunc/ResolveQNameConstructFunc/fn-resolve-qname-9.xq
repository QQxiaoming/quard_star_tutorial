(: Name: fn-resolve-qname-9 :)
(: Description: Evaluation of fn:resolve-qname function for which the given qname has no prefix and there is a namespace binding for the element. :)
(: uses fn:namespace-uri-from-qname to get namespace uri.  Element is given as direct element:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:namespace-uri-from-QName(fn:resolve-QName("anElement", <anElement xmlns="http://example.com/examples">Some content</anElement>)))