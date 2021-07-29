(: Name: fn-static-base-uri-6 :)
(: Description: Evaluation of fn:static-base-uri function using "tel". :)
(: Uses fn:string .:)

declare base-uri "tel:+1-816-555-1212";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())