(: Name: prefix-from-qname-2 :)
(: Description: Evaluation of fn-prefix-fromQName function with wrong argument type. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:prefix-from-QName(xs:integer(1))