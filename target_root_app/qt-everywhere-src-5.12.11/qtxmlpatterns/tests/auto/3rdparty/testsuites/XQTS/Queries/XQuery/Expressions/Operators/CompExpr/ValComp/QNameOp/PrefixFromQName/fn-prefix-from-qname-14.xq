(: Name: prefix-from-qname-14 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:string-length. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:prefix-from-QName(xs:QName("foo:bar")))