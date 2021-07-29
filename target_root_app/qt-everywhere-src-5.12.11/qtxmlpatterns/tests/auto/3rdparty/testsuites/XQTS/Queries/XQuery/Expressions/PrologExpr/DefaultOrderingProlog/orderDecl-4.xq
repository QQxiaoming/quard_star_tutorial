(: Name: orderDecl-4:)
(: Description: Simple ordering mode test.  Mode set to "ordered".  Use "intersect" operator.:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//overtime) intersect ($input-context1//overtime)