(: Name: whereClause-6 :)
(: Description: use of where clause used with a string value and fn:string :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := "String"
where fn:string($var) = "String"
return
 $var