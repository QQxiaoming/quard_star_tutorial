(: Name: fn-static-base-uri-13 :)
(: Description: Evaluation of fn:static-base-uri function using Used as argument to fn:substring-before function. :)
(: Uses fn:string .:)

declare base-uri "http://www.example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring-before(fn:string(fn:static-base-uri()),":")