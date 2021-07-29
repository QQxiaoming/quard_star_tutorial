(: Name: fn-string-2:)
(: Description: Evaluation of the fn:string function with argument set to the empty sequence.:)
(: Use fn:count to avoid empty file. :)
(: insert-start :)

declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:string(()))