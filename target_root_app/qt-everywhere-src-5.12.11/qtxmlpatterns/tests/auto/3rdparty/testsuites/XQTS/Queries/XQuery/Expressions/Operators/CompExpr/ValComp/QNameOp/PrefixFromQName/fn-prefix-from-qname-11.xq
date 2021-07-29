(: Name: prefix-from-qname-10 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:lower-case. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:concat(fn:prefix-from-QName(xs:QName("foo:bar")),":bar")