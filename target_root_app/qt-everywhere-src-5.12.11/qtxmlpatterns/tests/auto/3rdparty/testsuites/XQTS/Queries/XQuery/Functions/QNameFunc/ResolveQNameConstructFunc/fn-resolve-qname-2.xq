(: Name: fn-resolve-qname-2 :)
(: Description: Evaluation of fn:resolve-qname function for which the give qname has a prefix but element does not have binding namespace.:)
(: Element is give as a directly constructed element:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:resolve-QName("p1:anElement", <anElement>Some content</anElement>) 
