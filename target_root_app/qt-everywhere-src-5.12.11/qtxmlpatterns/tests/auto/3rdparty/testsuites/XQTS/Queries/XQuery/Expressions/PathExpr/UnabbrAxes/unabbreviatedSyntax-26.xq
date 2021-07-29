(: Name: unabbreviatedSyntax-26 :)
(: Description: Evaluate "/descendant::employee[fn:position() = 12]".  Selects the twelfth employee element in the document containing the context node. :)			  

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/descendant::employee[fn:position() = 12]