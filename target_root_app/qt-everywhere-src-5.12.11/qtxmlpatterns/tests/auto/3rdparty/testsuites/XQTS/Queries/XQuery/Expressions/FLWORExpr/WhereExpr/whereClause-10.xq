(: Name: whereClause-10 :)
(: Description: use of where clause used with quantified expression (some keyword). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := 1

where some $x in (1, 2) satisfies fn:string($var) = "1"
return
 fn:string($var )