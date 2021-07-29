(: Name: abbreviatedSyntax-10 :)
(: Description: Evaluates "//hours".  Selects all the hours descendants of the root document node and thus selects all hours elements in the same document as the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works) 
 return $h//hours