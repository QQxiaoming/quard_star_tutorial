(: Name: whereClause-8 :)
(: Description: use of where clause used with a fn:count() function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := 100
where fn:count(($var)) = 1
return
 fn:count(($var))