(: Name: abbreviatedSyntax-8 :)
(: Description: Evaluates "/works/employee[5]/hours[2]" selects the second hours of the fifth employee of the book whose parent is the document node that contains the context node. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/*/hours