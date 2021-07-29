(: Name: prefix-from-qname-12 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:substring-before. :)
declare namespace foo = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring-before(fn:prefix-from-QName(xs:QName("foo:bar")),"oo")