(: Name: fn-base-uri-5 :)
(: Description: Evaluation of base-uri function with argument set to a computed constructed comment :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(comment {"A Comment Node "}))