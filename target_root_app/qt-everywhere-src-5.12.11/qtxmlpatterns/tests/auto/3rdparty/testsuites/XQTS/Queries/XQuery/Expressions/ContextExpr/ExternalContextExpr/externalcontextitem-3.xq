(: Name: externalcontextitem-3 :)
(: Description: context item expression where context item is used as an integer. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/xs:integer(exactly-one(hours))
