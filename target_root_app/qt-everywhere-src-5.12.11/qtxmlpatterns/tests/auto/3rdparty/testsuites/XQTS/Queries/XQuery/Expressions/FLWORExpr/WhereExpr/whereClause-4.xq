(: Name: whereClause-4 :)
(: Description: use of where clause involving an "or" ("and" operator) expression. Use fn:false():)
(: Use fn:not() :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := (fn:false())
where fn:not($var and fn:false())
return
  $var and fn:false()