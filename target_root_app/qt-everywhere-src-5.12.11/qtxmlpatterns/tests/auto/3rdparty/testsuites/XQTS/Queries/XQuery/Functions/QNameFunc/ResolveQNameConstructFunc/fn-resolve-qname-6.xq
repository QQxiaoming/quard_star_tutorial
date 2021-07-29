(: Name: fn-resolve-qname-6 :)
(: Description: Evaluation of fn:resolve-qname function for which the give qname has a prefix and there is a namespace binding for the element. :)
(: Element has multiple namespapce bindings. Uses fn:local-name-from-QName to get local part.  Element is given as direct element:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:local-name-from-QName(fn:resolve-QName("p1:name", <anElement xmlns:p1="http://example.com/examples" xmlns:P1="http://example.com/examples">Some content</anElement>)))
