(: Name: nametest-18 :)
(: Description: Evaluation of a name test, which selects a child that was created using a declared namespace (qualified name). Use "child::*:b" syntax. :)

declare namespace ns1 = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><ns1:b attr2 = "abc2">context2</ns1:b></a>
return
 $var/child::*:b
