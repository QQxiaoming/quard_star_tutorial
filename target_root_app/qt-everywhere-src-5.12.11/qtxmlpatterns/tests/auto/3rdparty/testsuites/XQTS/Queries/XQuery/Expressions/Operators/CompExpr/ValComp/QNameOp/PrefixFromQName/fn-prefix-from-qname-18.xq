(: Name: prefix-from-qname-18 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:ends-with. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:ends-with(fn:prefix-from-QName(xs:QName("foo:bar")),"f")