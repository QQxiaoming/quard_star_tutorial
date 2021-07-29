(: Name: fn-base-uri-15 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed PI node argument. :)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(<?format role="output" ?>))