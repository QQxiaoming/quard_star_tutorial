(: Name: externalcontextitem-17 :)
(: Description: context item expression where context item used in a boolean (or operator)expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/(xs:boolean(exactly-one(hours) - 39) or xs:boolean(exactly-one(hours) - 39))
