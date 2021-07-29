(: Name: orderDecl-14:)
(: Description: Simple ordering mode test.  Mode set to "unordered" for an XPATH expression containing "/":)

declare ordering unordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in ($input-context1/works/employee[4]/hours)
 return $x