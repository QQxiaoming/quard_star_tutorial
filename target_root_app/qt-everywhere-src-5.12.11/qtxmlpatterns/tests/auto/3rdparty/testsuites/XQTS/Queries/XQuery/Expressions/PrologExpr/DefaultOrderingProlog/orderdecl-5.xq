(: Name: orderDecl-5:)
(: Description: Simple ordering mode test.  Mode set to "unordered".  Use "intersect" operator.:)

declare ordering unordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//overtime) intersect ($input-context1//overtime)