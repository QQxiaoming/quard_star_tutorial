(: Name: fn-resolve-qname-8 :)
(: Description: Evaluation of fn:resolve-qname function for which the given qname has no prefix and there is a namespace binding for the element. :)
(: uses fn:local-part-from-QName to get local part.  Element is given as direct element:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:local-name-from-QName(fn:resolve-QName("anElement", <anElement xmlns:p1="http://example.com/examples">Some content</anElement>)))