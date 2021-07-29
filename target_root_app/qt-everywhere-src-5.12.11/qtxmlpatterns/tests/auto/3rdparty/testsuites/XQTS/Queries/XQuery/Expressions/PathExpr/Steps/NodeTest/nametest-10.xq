(: Name: nametest-10 :)
(: Description: Name test that selects an "child::b" of a newly created element node and whose namespace URI is declared as the default namespace.:)

declare default element namespace "http://www.example.org/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><b attr2 = "abc2">context2</b></a>
return
  fn:namespace-uri(exactly-one($var/child::b))
