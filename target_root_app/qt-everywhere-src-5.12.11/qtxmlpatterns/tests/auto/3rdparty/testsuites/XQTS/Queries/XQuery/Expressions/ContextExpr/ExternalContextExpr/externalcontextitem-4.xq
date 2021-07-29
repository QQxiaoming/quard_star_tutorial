(: Name: externalcontextitem-4 :)
(: Description: context item expression where context item is used as a decimal. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/xs:decimal(exactly-one(hours))
