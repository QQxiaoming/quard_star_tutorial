(: Name: whereClause-7 :)
(: Description: use of where clause used with a string-length function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := "String"
where fn:string-length($var) = 6
return
 fn:string-length($var)