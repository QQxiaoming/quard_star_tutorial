(: Name: externalcontextitem-7 :)
(: Description: context item expression where context item is used as an xs:boolean. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/xs:boolean(exactly-one(hours) - 39)
