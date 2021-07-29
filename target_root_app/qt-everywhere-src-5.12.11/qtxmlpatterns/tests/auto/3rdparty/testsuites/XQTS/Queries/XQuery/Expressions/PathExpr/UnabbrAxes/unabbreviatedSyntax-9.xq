(: Name: unabbreviatedSyntax-9 :)
(: Description: Evaluate selecting a descendant (descendant::empnum)- Select the "empnum" descendants of the context node:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee) 
 return $h/descendant::empnum