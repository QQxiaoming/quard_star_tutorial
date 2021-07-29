(: Name: externalcontextitem-13 :)
(: Description: context item expression where context item used in mod operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/(exactly-one(hours) mod exactly-one(hours))
