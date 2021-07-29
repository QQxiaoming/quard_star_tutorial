(: Name: abbreviatedSyntax-12 :)
(: Description: Evaluates "//overtime/day".  Selects all the day elements in the same document as the context node that have an overtime parent. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h//overtime/day