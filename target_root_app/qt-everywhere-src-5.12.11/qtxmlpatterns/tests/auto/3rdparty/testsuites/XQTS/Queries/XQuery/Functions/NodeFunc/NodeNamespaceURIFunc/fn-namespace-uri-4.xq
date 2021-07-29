(: Name: fn-namespace-uri-4:)
(: Description: Evaluation of the fn:namespace-uri function argument set to empty sequence.:)
(: Use the fn:count function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri(()))
