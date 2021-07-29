(: Name: orderDecl-15:)
(: Description: Simple ordering mode test.  Mode set to "ordered" for an XPATH expression containing "//":)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $x in ($input-context1//day)
 return $x