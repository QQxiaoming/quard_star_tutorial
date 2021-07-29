(: Name: prefix-from-qname-9 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:upper-case. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:upper-case(fn:prefix-from-QName(xs:QName("foo:bar")))