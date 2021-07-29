(: Name: fn-static-base-uri-8 :)
(: Description: Evaluation of fn:static-base-uri function using Used as argument to upper-case function. :)
(: Uses fn:string .:)

declare base-uri "http://www.example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:upper-case(fn:string(fn:static-base-uri()))