(: Name: externalcontextitem-6 :)
(: Description: context item expression where context item is used sa a double. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/xs:double(exactly-one(hours))
