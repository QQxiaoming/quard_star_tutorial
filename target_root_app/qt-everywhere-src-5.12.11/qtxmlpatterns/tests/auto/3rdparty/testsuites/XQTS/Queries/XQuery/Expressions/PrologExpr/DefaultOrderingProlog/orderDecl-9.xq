(: Name: orderDecl-9:)
(: Description: Simple ordering mode test.  Mode set to "ordered" for a FLOWR expression with order modifier (descending).:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in $input-context1//hours
  order by $x descending return $x