(: Name: nametest-6 :)
(: Description: Name test that examines "ns1:b" for a newly constructed element that uses a declared namespace.:)

declare namespace ns1 = "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><ns1:b attr2 = "abc2">context2</ns1:b></a>
return
  $var/child::ns1:b