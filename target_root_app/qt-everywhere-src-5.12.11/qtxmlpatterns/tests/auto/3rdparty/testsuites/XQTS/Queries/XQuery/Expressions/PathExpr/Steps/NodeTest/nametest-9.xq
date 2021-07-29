(: Name: nametest-9 :)
(: Description: Name test that attempts to select non-existent nodes from a newly constructed.:)
(: Use fn:count to avoid empty file.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <a attr1 = "abc1"><b attr2 = "abc2">context2</b></a>
return
  fn:count(($var/empty-node-list)[1])