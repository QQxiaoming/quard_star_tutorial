(: Name: fn-resolve-uri-1 :)
(: Description: Evaluation of resolve-uri function with relative argument set to empty sequence :)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:resolve-uri((),"BaseValue"))