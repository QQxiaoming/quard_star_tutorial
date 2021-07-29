(: Name: fn-resolve-qname-3 :)
(: Description: Evaluation of fn:resolve-qname function for which the give qname is the empty sequence :)
(: uses fn:count to avoid empty file:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:resolve-QName((), <anElement>Some content</anElement>))
