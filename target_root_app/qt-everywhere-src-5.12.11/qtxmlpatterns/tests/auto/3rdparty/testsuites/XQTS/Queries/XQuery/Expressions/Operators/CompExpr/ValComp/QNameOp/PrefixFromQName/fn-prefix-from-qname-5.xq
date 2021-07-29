(: Name: prefix-from-qname-5 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to xs:string function and with prefix :)

declare namespace foo = "http://example.org";


(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:string(fn:prefix-from-QName(xs:QName("foo:name")))