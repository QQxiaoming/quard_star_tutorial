(: Name: fn-resolve-qname-1 :)
(: Description: Evaluation of fn:resolve-qname function for which the give qname is not lexically correct. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:resolve-QName("aName::", <anElement>Some content</anElement>) 
