(: Name: externalcontextitem-8 :)
(: Description: context item expression where context item is an xs:boolean used with fn:not(). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $var in ($input-context1/works/employee[1])
return $var/fn:not(xs:boolean(exactly-one(hours) - 39))
