(: Name: nametest-3 :)
(: Description: Name test that uses an unknown prefix. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><b attr2 = "abc2">context2</b></a>
return
  $var/child::pr:b
