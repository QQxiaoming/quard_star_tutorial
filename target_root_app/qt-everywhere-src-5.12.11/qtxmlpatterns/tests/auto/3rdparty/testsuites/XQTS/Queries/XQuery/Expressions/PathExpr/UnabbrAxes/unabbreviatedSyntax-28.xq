(: Name: unabbreviatedSyntax-28 :)
(: Description: Evaluate "child::employee[attribute::name eq "Jane Doe 11"]". Selects all "employee" children of the context node :)
(: that have a "name" attribute with value "Jane Doe 11". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee[attribute::name eq "Jane Doe 11"]