(: Name: fn-namespace-uri-7:)
(: Description: Evaluation of the fn:namespace-uri function argument set to a direct constructed processing instruction node.:)
(: Use the fn:count function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:namespace-uri(<?PI-Node PI-Content?>))
