(: Name: nametest-11 :)
(: Description: Name test that selects an "child::b" of a newly created element node and whose namespace URI is in no namespace.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><b attr2 = "abc2">context2</b></a>
return
  fn:count(fn:namespace-uri(exactly-one($var/child::b)))
