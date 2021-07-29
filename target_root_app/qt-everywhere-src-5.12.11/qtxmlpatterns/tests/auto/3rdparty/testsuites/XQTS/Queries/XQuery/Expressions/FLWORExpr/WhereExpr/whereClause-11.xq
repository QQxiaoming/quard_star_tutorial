(: Name: whereClause-11 :)
(: Description: use of where clause used with quantified expression (every keyword). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := 1

where every $x in (1, 1) satisfies fn:string($var) = "1"
return
 fn:string($var )