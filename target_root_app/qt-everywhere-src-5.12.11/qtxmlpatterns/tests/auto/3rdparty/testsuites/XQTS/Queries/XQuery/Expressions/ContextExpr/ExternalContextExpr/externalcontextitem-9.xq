(: Name: externalcontextitem-9 :)
(: Description: context item expression where context item used as argumet to fn:sum. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/fn:sum((hours,hours))