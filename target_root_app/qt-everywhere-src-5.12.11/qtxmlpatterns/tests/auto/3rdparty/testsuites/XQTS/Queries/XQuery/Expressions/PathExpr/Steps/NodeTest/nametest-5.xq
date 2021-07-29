(: Name: nametest-5 :)
(: Description: Name test that examines "*:b" for a newly constructed element.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><b attr2 = "abc2">context2</b></a>
return
  $var/child::*:b
