(: Name: unabbreviatedSyntax-29 :)
(: Description: Evaluate "child::employee[attribute::gender eq 'female'][fn:position() = 5]".  Selects the fifth employee child of the context node that has a gender attribute with value "female". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::employee[attribute::gender eq 'female'][fn:position() = 5]