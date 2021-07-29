(: Name: prefix-from-qname-4 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to xs:string function and no prefix :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(xs:string(fn:prefix-from-QName(xs:QName("name"))))