(: Name: prefix-from-qname-7 :)
(: Description: Evaluation of fn-prefix-fromQName function with argument set to empty sequence. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:prefix-from-QName(()))