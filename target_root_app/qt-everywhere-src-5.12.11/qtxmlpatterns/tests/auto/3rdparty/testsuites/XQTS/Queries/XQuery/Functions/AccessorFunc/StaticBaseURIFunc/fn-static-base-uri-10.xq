(: Name: fn-static-base-uri-10 :)
(: Description: Evaluation of fn:static-base-uri function using Used as argument to fn:concat function. :)
(: Uses fn:string .:)

declare base-uri "http://www.example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:concat(fn:string(fn:static-base-uri()),"another string")