(: Name: orderDecl-12:)
(: Description: Simple ordering mode test.  Mode set to "unordered" for a FLOWR expression with order modifier (ascending).:)

declare ordering unordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1//hours
   order by $x ascending return $x