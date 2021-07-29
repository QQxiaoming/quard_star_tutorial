(: Name: prefix-from-qname-17 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:starts-with. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:starts-with(fn:prefix-from-QName(xs:QName("foo:bar")),"f")