(: Name: orderDecl-7:)
(: Description: Simple ordering mode test.  Mode set to "unordered".  Use "except" operator.:)

declare ordering unordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//employee[1]) except ($input-context1//employee[2])