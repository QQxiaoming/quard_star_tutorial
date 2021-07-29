(: Name: unabbreviatedSyntax-33 :)
(: Description: Evaluate "child::*[self::empnum or self::pnum][fn:position() = fn:last()]".  Selects the last empnum or pnum child of the context node. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $h in ($input-context1/works/employee[6]) 
 return $h/child::*[self::empnum or self::pnum][fn:position() = fn:last()]