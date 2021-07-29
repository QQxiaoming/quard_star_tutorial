(: Name: unabbreviatedSyntax-32 :)
(: Description: Evaluate "child::*[self::pnum or self::empnum]".  Selects the pnum and empnum children of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[7]) 
 return $h/child::*[self::pnum or self::empnum]