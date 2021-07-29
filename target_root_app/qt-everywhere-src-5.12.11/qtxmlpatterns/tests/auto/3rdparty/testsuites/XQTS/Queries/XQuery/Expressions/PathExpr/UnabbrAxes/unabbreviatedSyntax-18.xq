(: Name: unabbreviatedSyntax-18 :)
(: Description: Evaluate /descendant::pnum selects all the pnum elements in the same document as the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/descendant::pnum