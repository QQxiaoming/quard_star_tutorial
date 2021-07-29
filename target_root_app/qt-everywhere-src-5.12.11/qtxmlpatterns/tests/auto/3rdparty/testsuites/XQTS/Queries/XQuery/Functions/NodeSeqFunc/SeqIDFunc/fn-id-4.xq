(: Name: fn-id-4 :)
(: Description: Evaluation of fn:id with node not being forom document where root is the document element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some Content</anElement>
return
  fn:id("argument1", $var)