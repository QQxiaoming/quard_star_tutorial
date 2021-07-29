(: Name: prefix-from-qname-3 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:string function and with prefix. :)

declare namespace foo = "http://example.org";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:prefix-from-QName(xs:QName("foo:name")))