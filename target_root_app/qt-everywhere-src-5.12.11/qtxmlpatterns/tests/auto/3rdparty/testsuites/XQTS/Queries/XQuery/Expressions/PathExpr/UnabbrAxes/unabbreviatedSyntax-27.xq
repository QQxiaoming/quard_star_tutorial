(: Name: unabbreviatedSyntax-27 :)
(: Description: Evaluate "/child::works/child::employee[fn:position() = 5]/child::hours[fn:position() = 2]". Selects the second hour of the fifth employee of the works whose parent is the document :)
(: node that contains the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1) 
 return $h/child::works/child::employee[fn:position() = 5]/child::hours[fn:position() = 2]