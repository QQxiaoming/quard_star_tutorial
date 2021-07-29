(: Name: externalcontextitem-21 :)
(: Description: context item expression where context item used with "max" function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/fn:max((hours,exactly-one(hours) + 1,22))
