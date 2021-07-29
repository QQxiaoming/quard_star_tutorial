(: Name: prefix-from-qname-13 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:substring-after. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring-after(fn:prefix-from-QName(xs:QName("foo:bar")),"f")