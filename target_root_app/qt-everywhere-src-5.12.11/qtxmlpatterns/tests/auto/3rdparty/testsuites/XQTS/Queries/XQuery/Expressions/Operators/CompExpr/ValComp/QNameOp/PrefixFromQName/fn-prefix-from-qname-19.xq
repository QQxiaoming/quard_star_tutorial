(: Name: prefix-from-qname-19 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:contains. :)

declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:contains(fn:prefix-from-QName(xs:QName("foo:bar")),"f")