(: Name: prefix-from-qname-15 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:substring. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring(fn:prefix-from-QName(xs:QName("foo:bar")),2)