(: Name: fn-base-uri-6 :)
(: Description: Evaluation of base-uri function with argument set to a computed constructed Text node. :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(text {"A Text Node"}))