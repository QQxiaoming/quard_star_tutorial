(: Name: externalcontextitem-19 :)
(: Description: context item expression where context item used with "avg" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)
for $var in ($input-context1/works/employee[1])
return $var/fn:avg((hours,hours,hours))