(: Name: orderDecl-6:)
(: Description: Simple ordering mode test.  Mode set to "ordered".  Use "except" operator.:)

declare ordering ordered;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

($input-context1//employee[1]) except ($input-context1//employee[2])