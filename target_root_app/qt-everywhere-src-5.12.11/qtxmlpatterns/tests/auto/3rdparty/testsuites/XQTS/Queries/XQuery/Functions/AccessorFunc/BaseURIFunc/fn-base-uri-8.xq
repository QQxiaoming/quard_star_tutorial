(: Name: fn-base-uri-8 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed Element node with not base-xml argument. :)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)
fn:count(fn:base-uri(<anElement>Element content</anElement>))