(: Name: whereClause-2 :)
(: Description: use of where clause involving an "or" expression. Use fn:false():)
(: Use fn:not() :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := (fn:false())
where fn:not($var or fn:false())
return
  $var or fn:false()