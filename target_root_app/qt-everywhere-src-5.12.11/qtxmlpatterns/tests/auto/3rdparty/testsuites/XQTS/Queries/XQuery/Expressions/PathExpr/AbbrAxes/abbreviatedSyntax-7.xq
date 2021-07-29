(: Name: abbreviatedSyntax-7 :)
(: Description: Evaluates "*/hours". Selects all hours grandchildren of the context node. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/*/hours