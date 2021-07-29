(: Name: externalcontextitem-5 :)
(: Description: context item expression where context item isused as a float. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in $input-context1/works/employee[1]
return $var/xs:float(exactly-one(hours))
