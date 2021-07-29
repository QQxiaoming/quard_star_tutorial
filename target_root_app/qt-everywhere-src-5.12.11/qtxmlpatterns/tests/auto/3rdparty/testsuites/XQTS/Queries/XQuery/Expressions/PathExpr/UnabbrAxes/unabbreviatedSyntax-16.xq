(: Name: unabbreviatedSyntax-16 :)
(: Description: Evaluate child::*/child::pnum - Selects the "pnum" grandchildren of the context node:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/child::*/child::pnum