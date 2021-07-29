(: Name: prefix-from-qname-10 :)
(: Description: Evaluation of fn-prefix-fromQName function as argument to fn:lower-case. :)
declare namespace FOO = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lower-case(fn:prefix-from-QName(xs:QName("FOO:bar")))