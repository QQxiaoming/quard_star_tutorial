(: Name: fn-resolve-qname-10 :)
(: Description: Evaluation of fn:resolve-qname function for which the given qname has no prefix and there is a namespace binding with prefix for the element. :)
(: uses fn:namespace-uri-from-qname to get namespace uri.  Element is given as direct element:)
(: use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri-from-QName(fn:resolve-QName("anElement", <anElement xmlns:p1="http://example.com/examples">Some content</anElement>)))