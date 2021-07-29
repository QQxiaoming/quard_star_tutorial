(: Name: fn-idref-3 :)
(: Description: Evaluation of fn:idref with second argument not a node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := "A"
return
  fn:idref("argument1", $var)