(: Name: externalcontextitem-2 :)
(: Description: context item expression where context item is used as string. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in $input-context1/works/employee[1]
return $var/xs:string(exactly-one(empnum))
