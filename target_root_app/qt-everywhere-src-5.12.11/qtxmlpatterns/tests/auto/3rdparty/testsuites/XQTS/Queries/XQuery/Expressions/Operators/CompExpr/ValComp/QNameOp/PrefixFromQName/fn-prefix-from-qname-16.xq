(: Name: prefix-from-qname-16 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:string-join. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-join((fn:prefix-from-QName(xs:QName("foo:bar")),":bar"),"")