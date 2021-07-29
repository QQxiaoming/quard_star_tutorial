(: Name: prefix-from-qname-20 :)
(: Description: Evaluation of fn-prefix-fromQName function with two namespaces declared with same namespace (different case). :)

declare namespace foo = "http://example.org";
declare namespace FOO = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:prefix-from-QName(xs:QName("foo:bar"))