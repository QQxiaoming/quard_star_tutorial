(: Name: orderDecl-8:)
(: Description: Simple ordering mode test.  Mode set to "ordered" for a FLOWR expression with no ordering mode.:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1//hours
    return $x