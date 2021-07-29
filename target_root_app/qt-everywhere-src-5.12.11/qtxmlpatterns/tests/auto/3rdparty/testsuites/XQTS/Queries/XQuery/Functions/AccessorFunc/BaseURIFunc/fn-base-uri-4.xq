(: Name: fn-base-uri-4 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed comment :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(<!-- A comment -->))