(: Name: fn-static-base-uri-3 :)
(: Description: Evaluation of fn:static-base-uri function using "mailto". :)
(: Uses fn:string .:)

declare base-uri "mailto:John.Doe@example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())