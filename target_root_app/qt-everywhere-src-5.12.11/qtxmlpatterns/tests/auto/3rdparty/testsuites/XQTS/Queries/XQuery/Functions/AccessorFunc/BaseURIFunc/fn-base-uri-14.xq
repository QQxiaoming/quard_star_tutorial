(: Name: fn-base-uri-14 :)
(: Description: Evaluation of base-uri function with argument set to a computed constructed attribute node argument. :)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(attribute anAttribute{"attribute value"}))
