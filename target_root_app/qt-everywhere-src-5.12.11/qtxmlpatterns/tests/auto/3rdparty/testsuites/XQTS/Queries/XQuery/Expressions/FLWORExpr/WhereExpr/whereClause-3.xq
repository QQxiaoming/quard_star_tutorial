(: Name: whereClause-3 :)
(: Description: use of where clause involving an "or" ("and" operator) expression. Use fn:true():)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := (fn:true())
where $var and fn:true()
return
  $var and fn:true()