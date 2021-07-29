(: Name: nametest-1 :)
(: Description: Name test that evaluates a child "b" of a newly construted node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><b attr2 = "abc2">context2</b></a>
return
  $var/child::*
