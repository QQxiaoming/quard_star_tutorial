(: Name: prefix-from-qname-8 :)
(: Description: Evaluation of fn-prefix-fromQName function with a prefix that is not defined. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:prefix-from-QName(xs:QName("foo:bar"))