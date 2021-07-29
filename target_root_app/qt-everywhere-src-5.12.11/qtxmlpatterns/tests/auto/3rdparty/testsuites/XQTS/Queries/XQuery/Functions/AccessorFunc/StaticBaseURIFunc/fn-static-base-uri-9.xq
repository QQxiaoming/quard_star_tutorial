(: Name: fn-static-base-uri-9 :)
(: Description: Evaluation of fn:static-base-uri function using Used as argument to lower-case function. :)
(: Uses fn:string .:)

declare base-uri "http://www.example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lower-case(fn:string(fn:static-base-uri()))