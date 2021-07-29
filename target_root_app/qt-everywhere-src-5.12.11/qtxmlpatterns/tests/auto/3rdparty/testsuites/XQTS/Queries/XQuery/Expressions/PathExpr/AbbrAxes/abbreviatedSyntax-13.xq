(: Name: abbreviatedSyntax-13 :)
(: Description: Evaluates ".//day".  Selects the day element descendants of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h/.//day