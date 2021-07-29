(: Name: fn-base-uri-3 :)
(: Description: Evaluation of base-uri function with argument set to empty sequence :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(()))