(: Name: prefix-from-qname-3 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:string function and no prefix. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:prefix-from-QName(xs:QName("name")))