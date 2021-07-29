(: Name: fn-name-1:)
(: Description: Evaluation of the fn:name function with the argument set to the empty sequence.:)
(: Uses the fn:string-length function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:name(()))
