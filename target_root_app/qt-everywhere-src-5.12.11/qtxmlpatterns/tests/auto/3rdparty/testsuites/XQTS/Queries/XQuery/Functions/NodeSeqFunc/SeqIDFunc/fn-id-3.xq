(: Name: fn-id-3 :)
(: Description: Evaluation of fn:id with second argument not a node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := "A"
return
  fn:id("argument1", $var)