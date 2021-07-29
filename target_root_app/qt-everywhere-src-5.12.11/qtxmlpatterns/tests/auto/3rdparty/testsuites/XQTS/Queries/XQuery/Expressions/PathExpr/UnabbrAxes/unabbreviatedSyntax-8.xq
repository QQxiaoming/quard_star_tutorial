(: Name: unabbreviatedSyntax-8 :)
(: Description: Evaluate selecting the parent (parent::node()) of the context node:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[1]/hours) 
 return $h/parent::node()