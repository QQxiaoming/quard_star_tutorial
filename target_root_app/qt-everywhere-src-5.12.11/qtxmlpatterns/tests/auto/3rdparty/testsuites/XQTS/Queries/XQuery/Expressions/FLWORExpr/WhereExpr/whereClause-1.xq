(: Name: whereClause-1 :)
(: Description: use of where clause involving an "or" expression. Use fn:true():)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := (fn:true())
where $var or fn:true()
return
  $var or fn:true()