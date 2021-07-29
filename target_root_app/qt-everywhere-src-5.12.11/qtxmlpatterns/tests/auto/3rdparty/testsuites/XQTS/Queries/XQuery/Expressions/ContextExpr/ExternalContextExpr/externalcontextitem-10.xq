(: Name: externalcontextitem-10 :)
(: Description: context item expression where context item used in addition operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $var in ($input-context1/works/employee[1])
return $var/(exactly-one(hours) + exactly-one(hours))
