(: Name: externalcontextitem-20 :)
(: Description: context item expression where context item used with "min" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/fn:min((hours,hours,22))