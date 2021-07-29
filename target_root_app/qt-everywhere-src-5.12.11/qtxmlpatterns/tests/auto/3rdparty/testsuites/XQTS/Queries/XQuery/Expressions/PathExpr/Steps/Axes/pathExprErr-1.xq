(: Name: pathExprErr-1:)
(: Description: Evaluation of of "/" with context item not being a node..:)


(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := "100"
return
  $var/child::text()