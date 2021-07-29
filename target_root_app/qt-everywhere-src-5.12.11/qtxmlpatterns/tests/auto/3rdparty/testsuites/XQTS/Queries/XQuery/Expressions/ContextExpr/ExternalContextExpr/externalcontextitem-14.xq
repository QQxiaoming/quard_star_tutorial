(: Name: externalcontextitem-14 :)
(: Description: context item expression where context item used in division (div operator) operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/(exactly-one(hours) div exactly-one(hours))
